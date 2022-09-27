#pragma once

#include <cstddef>

#include "../base.h"

namespace estd { namespace experimental {

// regular memory_pool.h seems to work well, but was done very long ago and I can't remember
// enough about its underlying architecture to proceed comfortably.
namespace memory { namespace v2 {

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

}}

}}