#pragma once

#include <cstddef>

#include "../internal/deduce_fixed_size.h"
#include "../memory.h"  // for platform + raw_instance_provider
#include "../array.h"
#include "../algorithm.h"
#include <bitset>

namespace estd { namespace experimental {

// regular memory_pool.h seems to work well, but was done very long ago and I can't remember
// enough about its underlying architecture to proceed comfortably.
namespace v2 {

enum class pool_types
{
    linked_list,
    handle
};

template <class T, std::size_t sz, pool_types type>
class memory_pool;

template <std::size_t N>
struct memory_pool_base
{
    typedef typename internal::deduce_fixed_size_t<N>::size_type size_type;
};


template <class T, std::size_t N>
class memory_pool<T, N, pool_types::linked_list>
{
private:
    union item
    {
        item* next;
        // NOTE: Usage of estd::array may implicitly bring along
        // raw_instance_provider behavior.
        estd::experimental::raw_instance_provider<T> value;
    };

    estd::array<item, N> items;
    item* head;

public:
    typedef T value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    memory_pool()
    {
        item* i = items.begin();

        head = i;

        while(i != items.end())
        {
            // due to
            // https://stackoverflow.com/questions/26351162/will-right-hand-side-of-an-expression-always-evaluated-first
            // can't quite do a one-liner
            //i->next = ++i;

            item* temp = i;
            temp->next = ++i;
        }
    }

    // TODO: Do constructor flavor
    value_type* allocate()
    {
        if(head == items.end()) return NULLPTR;

        item* i = head;

        head = head->next;

        return &i->value.value();
    }

    void deallocate(value_type* value)
    {
        item* i = reinterpret_cast<item*>(value);

        i->next = head;
        head = i;
    }

    std::size_t count_free()
    {
        item* i = head;
        std::size_t count = 0;

        // TODO: Make a estd::accumulate
        for(; i != items.end(); ++i, ++count);

        return count;
    }

    // somewhat internal call used mainly down the line for aggregated pools,
    // does not indicate a valid pointer, only that the pointer does reside in
    // the pool range
    bool within_range(value_type* value)
    {
        return value >= items.begin() && value < items.end();
    }
};

/*
 *  perhaps not needed if above can conform to allocator signature/concept
 *  https://en.cppreference.com/w/cpp/memory/allocator
template <class T, std::size_t N, pool_types type>
class pool_allocator
{
private:
    memory_pool<T, N, type> pool;

public:

}; */

enum maintenance_mode
{
    uninitialized,
    compact,
    move
};


// Doesn't belong in memory_pool area - we can get away with it for now since it's all experimental code
template <unsigned N, unsigned N_Handles = (N >> 6)>
struct virtual_memory
{
    maintenance_mode mode = maintenance_mode::uninitialized;

    maintenance_mode state() const { return mode; }

    typedef unsigned char byte_type;
    typedef unsigned size_type;
    typedef unsigned handle_index;
    typedef byte_type* pointer;
    typedef unsigned index_type;

    byte_type storage[N];

    struct handle
    {
        pointer data;
        size_type chunk_size;

        handle() { data = NULLPTR; }
    };

    // available space
    struct item
    {
        item* next;
        // tracks the size of free memory, including space occupied by this item
        size_type chunk_size;
    };

    // occupied space
    handle handles[N_Handles];

    item* free_head;

    union
    {
        struct
        {
            item* evaluating;
            item* evaluating_prev;
            size_type items_processed;

        } compact_state;

        struct
        {

        } move_state;
    };


    virtual_memory()
    {
        free_head = new (&storage[0]) item;
        free_head->next = NULLPTR;
        free_head->chunk_size = N;
    }

    handle* get_free_handle(index_type& index)
    {
        for(index_type i = 0; i < N_Handles; i++)
        {
            handle& h = handles[i];

            if(h.data == NULLPTR)
            {
                index = i;
                return &h;
            }
        }

        return NULLPTR;
    }

    handle_index allocate(size_type size)
    {
        item* i = free_head,
            *prev;

        // TODO: coerce size to be aligned so that allocations and frees also have aligned size

        // minimum size is 'item' size so that fragments have enough space to track themselves
        if(size < sizeof(item))
            size = sizeof(item);

        while(i != NULLPTR)
        {
            handle_index hi;
            if(i->chunk_size == size)
            {
                prev->next = i->next;

                handle* h = get_free_handle(hi);
                h->data = (pointer) i;
                h->chunk_size = size;

                return hi;
            }
            else if(i->chunk_size > size)
            {
                handle* h = get_free_handle(hi);
                h->data = ((pointer) i) + (i->chunk_size - size);
                h->chunk_size = size;

                i->chunk_size -= size;

                return hi;
            }
            else
            {

            }

            prev = i;
            i = i->next;
        }

        return -1;
    }

    void free(index_type h_index)
    {
        handle& h = handles[h_index];

        item* i = (item*)h.data;

        i->chunk_size = h.chunk_size;
        i->next = free_head;

        h.data = NULLPTR;

        free_head = i;
    }

    size_type total_free(size_type* node_counter = NULLPTR) const
    {
        item* i = free_head;
        size_type count = 0;

        if(node_counter != NULLPTR) *node_counter = 0;

        while(i != NULLPTR)
        {
            if(node_counter != NULLPTR) ++(*node_counter);

            count += i->chunk_size;
            i = i->next;
        }

        return count;
    }

    bool maint_absorb(item* i_physical_0, item* i_physical_1)
    {
        // if physical ptr diff is same as item# 1 chunk size
        if((pointer)i_physical_1 - (pointer)i_physical_0 == i_physical_0->chunk_size)
        {
            ++compact_state.items_processed;

            // do merge/absorb operation

            i_physical_0->next = i_physical_1->next;
            i_physical_0->chunk_size += i_physical_1->chunk_size;

            return true;
        }
        else
            return false;
    }

    /// Walk the heap and bubble sort 'next' pointers which are out of order
    /// and if we encounter contiguous free space along the way, merge 'item' together
    bool maint_compact()
    {
        item* i = compact_state.evaluating;
        item* i_prev = compact_state.evaluating_prev;

        if(i->next == NULLPTR) return false;

        // if next free node is positioned before this one
        if(i->next < i)
        {
            ++compact_state.items_processed;

            item* i_physical_0 = i->next;
            item* i_physical_1 = i;
            // physical #2 may or may not be physically after #1, but we'll
            // decide that on the next iteration of maint_compact
            item* i_physical_2_assumed = i_physical_0->next;

            i_physical_0->next = i_physical_1;
            i_physical_1->next = i_physical_2_assumed;

            if(i_prev == NULLPTR)
            {
                // this implies i is free_head
                free_head = i_physical_0;
            }
            else
                i_prev->next = i_physical_0;

            if(maint_absorb(i_physical_0, i_physical_1))
                i = i_physical_1;
        }
        else
        {
            item* i_physical_0 = i;
            item* i_physical_1 = i->next;

            if(maint_absorb(i_physical_0, i_physical_1))
                i = i_physical_1;
        }

        compact_state.evaluating_prev = i;
        compact_state.evaluating = i->next;

        return true;
    }

    void maint_move()
    {

    }

    void maint()
    {
        switch(mode)
        {
            case maintenance_mode::uninitialized:
                mode = maintenance_mode::compact;
                compact_state.evaluating = free_head;
                compact_state.evaluating_prev = NULLPTR;
                mode = maintenance_mode::compact;
                maint();
                break;

            case maintenance_mode::compact:
                compact_state.items_processed = 0;
                maint_compact();
                if(compact_state.items_processed == 0)
                {
                    mode = maintenance_mode::move;
                }
                break;

            case maintenance_mode::move:
                maint_move();
                mode = maintenance_mode::uninitialized;
                break;
        }
    }
};

}

namespace v3 {

template <std::size_t N>
class virtual_memory
{
    typedef unsigned char byte_type;
    typedef byte_type* pointer;
    typedef unsigned size_type;
    typedef uint8_t handle_type;

    byte_type pool[N];

public: // Just for unit tests, otherwise this would be private

    struct flags
    {
        uint16_t handle : 6;
        uint16_t lock_counter : 4;
        uint16_t is_allocated : 1;

        // number of 'extra' bytes padded on the end of this block
        uint16_t padding: 4;

        // locked during maintenance tasks which touch this particular item
        uint16_t system_locked : 1;
    };

    struct item
    {
        item* next;
        flags flags_;

        handle_type handle() const { return flags_.handle; }

        bool is_allocated() const { return flags_.is_allocated; }
    };

    struct allocated_item : item
    {
        byte_type data[];

        allocated_item()
        {
            this->flags_.is_allocated = 1;
            this->flags_.lock_counter = 0;
        }

        allocated_item(handle_type h)
        {
            this->flags_.is_allocated = 1;
            this->flags_.lock_counter = 0;
            this->flags_.handle = h;
        }
    };

    struct free_item : item
    {
        free_item()
        {
            this->flags_.is_allocated = 0;
        }
    };

    struct item_iterator
    {
        typedef item value_type;
        typedef item* pointer;
        typedef item_iterator iterator;

        item* current;

        item_iterator(item* current) : current(current) {}


        iterator& operator++()
        {
            current = current->next;
            return *this;
        }

        value_type& operator*() const
        {
            return *current;
        }

        pointer operator->() const
        {
            return current;
        }
    };

    struct free_iterator
    {
        typedef free_item value_type;
        typedef free_item* pointer;
        typedef free_iterator iterator;

        pointer current;

        free_iterator(pointer current) : current(current) {}


        iterator& operator++()
        {
            item* i = current;

            while(i != NULLPTR && !i->is_allocated())
            {
                i = i->next;
            }

            current = (pointer)i;

            return *this;
        }

        iterator operator++(int)
        {
            iterator i(current);
            this->operator++();
            return i;
        }

        value_type& operator*() const
        {
            return *current;
        }

        pointer operator->() const
        {
            return current;
        }

        bool operator !=(const iterator& compare_to) const
        {
            return current != compare_to.current;
        }
    };

    // We compute size of tracked memory block based on next pointer
    // Also we subtract out overhead of item metadata
    size_type block_size(const item* i) const
    {
        size_type diff;
        if(i->next)
            diff = (byte_type*)i->next - (byte_type*)i;
        else
            diff = &pool[N] - (byte_type*)i;

        return diff - sizeof(item);
    }

    estd::span<byte_type> block(const allocated_item* i)
    {
        return estd::span<byte_type>(i->data, block_size(i));
    }

    item_iterator first_iterator()
    {
        return item_iterator(first);
    }

    item* first()
    {
        return reinterpret_cast<item*>(pool);
    }

    const item* first() const
    {
        return reinterpret_cast<const item*>(pool);
    }

    free_item* first_free()
    {
        for (item* i = first(); i != NULLPTR; i = i->next)
        {
            if(i->flags_.is_allocated == 0) return (free_item*)i;
        }

        return NULLPTR;
    }

    const free_item* first_free() const
    {
        for (const item* i = first(); i != NULLPTR; i = i->next)
        {
            if(i->flags_.is_allocated == 0) return (const free_item*)i;
        }

        return NULLPTR;
    }

    const free_iterator end() const
    {
        return free_iterator(NULLPTR);
    }

    size_type free_count() //const
    {
        free_iterator i(first_free());
        size_type counter = 0;

        while(i++ != end())
        {
            ++counter;
        }

        return counter;
    }

    ///
    /// \param i - updates next with new memory block between end of 'i' and beginning of old 'i->next'
    /// \param size
    /// \return
    free_item* split(item* i, size_type size)
    {
        free_item* new_split_item = new ((pointer) i + size + sizeof(item)) free_item();

        new_split_item->next = i->next;
        i->next = new_split_item;
        return new_split_item;
    }

    void merge_next(item* i)
    {
        i->next = i->next->next;
    }

    void opportunistic_merge(item* i)
    {
        item* root = i;

        while(i->next != NULLPTR && !i->next->is_allocated())
        {
            i = i->next;
        }

        root->next = i;
    }

    void defrag1(size_type min_, size_type max_)
    {
        item* i = first();

        item* preceding = NULLPTR;
        allocated_item* fragment = NULLPTR;

        for(; i != NULLPTR; preceding = i, i = i->next)
        {
            if(preceding != NULLPTR && !preceding->is_allocated())
            {
                if(i->is_allocated())
                {
                    if(i->next != NULLPTR && !i->next->is_allocated())
                    {
                        // we have a possible fragment, a free-allocated-free pattern

                        size_type bs = block_size(i);

                        if(bs >= min_ && bs <= max_)
                        {
                            min_ = bs;
                            fragment = (allocated_item*)i;
                            break;
                        }
                    }
                }
            }
        }

        if(fragment == NULLPTR) return;

        free_item* free_closest_to_min = NULLPTR;
        free_item* free_closest_to_max = NULLPTR;
        //allocated_item* alloc_closest_to_min = NULLPTR;
        //allocated_item* alloc_closest_to_max = NULLPTR;

        i = first();

        for(; i != NULLPTR; i = i->next)
        {
            size_type bs = block_size(i);

            if(i->is_allocated())
            {
                /*
                if(alloc_closest_to_max == NULLPTR)
                {
                    alloc_closest_to_min = i;
                    alloc_closest_to_max = i;
                }
                else
                {
                    if(bs < max_ && bs > block_size(alloc_closest_to_max))
                        alloc_closest_to_max = i;
                    if(bs > min_ && bs < block_size(alloc_closest_to_min))
                        alloc_closest_to_min = i;
                } */
            }
            else
            {
                if(free_closest_to_max == NULLPTR)
                {
                    free_closest_to_min = (free_item*)i;
                    free_closest_to_max = (free_item*)i;
                }
                else
                {
                    if(bs < max_ && bs > block_size(free_closest_to_max))
                        free_closest_to_max = (free_item*)i;
                    if(bs > min_ && bs < block_size(free_closest_to_min))
                        free_closest_to_min = (free_item*)i;
                }
            }
        }

        if(free_closest_to_min != NULLPTR)
        {
            // consider moving fragment to this free block
            if((pointer)free_closest_to_min > (pointer)fragment)
                reallocate_forward(fragment, free_closest_to_min);
            else
                reallocate_backward(fragment, free_closest_to_min);

            // careful, because fragment and free_closest_to_min now point to swapped and therefore
            // somewhat invalid data.
            if(!free_closest_to_min->next->is_allocated())
                opportunistic_merge(free_closest_to_min->next);
        }
    }

    CONSTEXPR static handle_type bad_handle()
    {
        return estd::numeric_limits<handle_type>::max();
    }

    item* find(handle_type h)
    {
        for(item* i = first(); i != NULLPTR; i = i->next)
        {
            if(i->handle() == h) return i;
        }

        return NULLPTR;
    }

    item* find_prev(item* _i)
    {
        item* i = first();

        if(_i == i) return NULLPTR;

        for(; i != NULLPTR; i = i->next)
        {
            if(i->next == _i) return i;
        }

        return NULLPTR;
    }

    handle_type find_lowest_new_handle_number()
    {
        std::bitset<64> available(0xFF); // matches up to handles : 6 bit field

        for(item* i = first(); i != NULLPTR; i = i->next)
        {
            available[i->flags_.handle] = 0;
        }

        for(handle_type i = 0; i < 64; i++)
        {
            if(available.test(i))
                return i;
        }

        return bad_handle();
    }

    void reallocate_backward(allocated_item* alloc_i, free_item* free_i)
    {
        pointer move_to = ((allocated_item*)free_i)->data;
        size_type size = block_size(alloc_i);

        std::swap(alloc_i->flags_, free_i->flags_);
        std::move_backward(alloc_i->data, alloc_i->data + size, move_to);

        // TODO: Need to do a split operation here against free_i
    }

    // more or less swaps allocated and unallocated blocks,
    // and updates preceding pointers
    void reallocate_forward(
        allocated_item* alloc_i, free_item* free_i)
    {
        pointer move_to = ((allocated_item*)free_i)->data;
        size_type size = block_size(alloc_i);

        std::swap(alloc_i->flags_, free_i->flags_);
        std::move(alloc_i->data, alloc_i->data + size, move_to);

        // TODO: Need to do a split operation here against free_i
    }

    // NOTE: Won't catch instances where i jumps around and a later i precedes a former i in some cases
    bool check_integrity() const
    {
        for(const item* i = first(); i != NULLPTR; i = i->next)
        {
            if(i->next != NULLPTR)
            {
                if(i->next < i)
                    return false;
            }
        }

        return true;
    }

public:
    // Overhead of a memory chunk, mainly used for testing purposes
    static CONSTEXPR size_type item_size() { return sizeof(item); }

    virtual_memory()
    {
        free_item* i = new (pool) free_item();

        i->flags_.handle = 0;
        i->next = NULLPTR;
    }

    size_type available() const
    {
        const item* i = first();
        size_type free_space = 0;

        do
        {
            if(!i->flags_.is_allocated)
                free_space += block_size(i);
        }
        while((i = i->next) != NULLPTR);

        return free_space;
    }

    void free(handle_type h)
    {
        // FIX: Undefined behavior if handle invalid
        free_item* i = (free_item*)find(h);

        i->flags_.is_allocated = 0;
        if(i->next != NULLPTR && !i->next->is_allocated())
        {
            merge_next(i);
        }
    }

    handle_type allocate(size_type size)
    {
        for(item* i = first(); i != NULLPTR; i = i->next)
        {
            size_type block_size_ = block_size(i);
            if(!i->flags_.is_allocated && block_size_ >= size)
            {
                ((free_item*)i)->~free_item();

                // handle itself is carried over from when it was a free block
                // so be careful not to blast that in ~free_item
                new (i) allocated_item();

                // If there's enough space to split off a new free memory block, do
                // so.  Otherwise, there will be lingering unused bytes less than
                // the size of an item as a kind of padding.  That might end up being
                // a baked-in fragmentation that we don't want, perhaps obviating this
                // technique
                if(block_size_ > size + sizeof(item))
                {
                    item* split_ = split(i, size);
                    split_->flags_.handle = find_lowest_new_handle_number();
                }
                else
                {
                    // FIX: Do actual calcuation here.  This is just placeholder code
                    i->flags_.padding = 1;
                }

                return i->handle();
            }
        }

        return estd::numeric_limits<handle_type>::max();
    }

    estd::span<byte_type> lock_span(handle_type h)
    {
        item* i = find(h);

        // FIX: Undefined behavior if h points to an unallocated block
        return estd::span<byte_type>(((allocated_item*)i)->data, block_size(i));
    }
};

}

}}