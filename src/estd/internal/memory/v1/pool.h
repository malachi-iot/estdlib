#pragma once

// DEBT: Temporarily using 'internal' only as we bring it up
namespace estd { namespace internal { namespace memory { inline namespace v1 {

// Lift direct from PGMEM-2, PGMEM-5
// Rudimentary memory pool.  Does not support:
// - querying how many blocks are allocated
// - pairing contiguous blocks together
// - fine tuned alignment on typed_pool

class pool_core
{
protected:
    union control
    {
        control* next_;
        char value_[1];

        /// Low level call - treats 'control' as a member of a free list
        /// Assigns control->next to what head is pointing to, then reassigns
        /// head to point to control
        void free(control** head)
        {
            next_ = *head;
            *head = this;
        }
    };

    control* head_free_;

    /// Low level call - assumes there is a free block
    control* alloc_next()
    {
        // grabs head of list to allocate it
        control* allocated = head_free_;

        head_free_ = head_free_->next_;

        return allocated;
    }

    constexpr explicit pool_core(control* head_free) :
        head_free_{head_free}
    {}

    const control* head() const { return head_free_; }
public:
    constexpr bool full() const { return head_free_ == nullptr; }

    unsigned available_blocks() const
    {
        unsigned size = 0;

        for(const control* i = head_free_; i != nullptr; i = i->next_)
            ++size;

        return size;
    }

    // It is presumed this block belongs to this pool.  Fun fact: if it doesn't, you can actually
    // 'free' external memory to this pool thus growing its size.  Undefined behavior, but likely
    // possible to make that an official feature
    void free(void* block)
    {
        auto c = reinterpret_cast<control*>(block);

        c->free(&head_free_);
    }

    void* alloc()
    {
        if(full())   return nullptr;

        // NOTE: value_ is symbolic for benefit of us the maintainer.  We in fact
        // could return the control* directly
        return alloc_next()->value_;
    }
};

// Block size known at compile time
template <unsigned block_size_>
class pool_block : public pool_core
{
    using base_type = pool_core;

protected:
    // NOTE: This must overlap perfectly with PoolCore::Control.  We DO NOT
    // inherit because that would goof up our union
    union control
    {
        base_type::control base_;

        // Allocated block
        // It's a feature not a bug that this overlaps with base - we use
        // space to its fullest when it's allocated
        char value_[block_size_];
    };

    // Work backwards and initialize forward list as empty blocks
    static void reset(control* const storage, unsigned blocks)
    {
        control* i = storage + blocks;
        base_type::control* current = nullptr;

        // Going backwards, start with next = null (current)
        // then free each one until we land at the start
        while(i-- > storage)
            i->base_.free(&current);
    }

    pool_block(control* head_free, unsigned blocks_to_init) :
        base_type(&head_free->base_)
    {
        //assert(blocks_to_init > 0);

        reset(head_free, blocks_to_init);
    }

public:
    static constexpr unsigned block_size()
    {
        return block_size_;
    }
};

template <unsigned block_size, unsigned blocks>
class pool : public pool_block<block_size>
{
    using base_type = pool_block<block_size>;
    using typename base_type::control;

    control storage_[blocks];

public:
    pool() : base_type(storage_, blocks)    {}

    static constexpr unsigned max_blocks() { return blocks; }

    int8_t to_handle(void* value) const
    {
        return ((control*)value - storage_);
    }

    void* from_handle(int8_t handle)
    {
        return &storage_[handle];
    }
};

template <class T, unsigned blocks>
class typed_pool : public pool<sizeof(T), blocks>
{
    using base_type = pool<sizeof(T), blocks>;

    // Purposefully hide non-typed alloc/free
    void alloc() {}
    void free() {}

public:
    template <class ...Args>
    T* emplace(Args&&...args)
    {
        void* const storage = base_type::alloc();

        if(storage == nullptr)  return nullptr;

        return new (storage) T(std::forward<Args>(args)...);
    }

    void erase(T* t)
    {
        t->~T();
        base_type::free(t);
    }

    T* from_handle(int8_t handle)
    {
        return (T*)base_type::from_handle(handle);
    }
};

}}}}