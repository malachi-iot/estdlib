#pragma once

#include "estd/memory.h"

// reference allocator for inbuild mechanisms.  basically a crummy test-only
// version of std new_allocator
template <class T>
class _allocator
{
    NODATA_MOTIVATOR;

public:
    // Would like to use this somehow to enable/disable auto-unlocking
    // for iterators
    static constexpr bool is_locking() { return false; }

    typedef typename estd::nothing_allocator<T>::lock_counter lock_counter;

    // holds the data (if necessary) to reveal the size of an allocation
    // some allocators innately have this, some don't.  near as I can tell,
    // standard malloc does not
    // As noted here https://stackoverflow.com/questions/1281686/determine-size-of-dynamically-allocated-memory-in-c
    // C doesn't provide a standard way to track this
    struct allocated_size_helper
    {
        size_t allocated_size;
    };

    typedef T value_type;
    typedef value_type* pointer;
    typedef const void* const_void_pointer;
    typedef value_type* handle_type;
    typedef void* handle_offset_type;

    class handle_with_offset
    {
        handle_offset_type loc;

    public:
        handle_with_offset(handle_offset_type loc) : loc(loc) {}

        T& lock() { return * (T*) loc; }

        void unlock() {}
    };

    static handle_type invalid() { return NULLPTR; }

    pointer lock(handle_type h) { return h; }

    void unlock(handle_type) { }

    pointer allocate(size_t size)
    {
        // FIX: unsure if I should be doing a 'new' here or not
        return (pointer) malloc(size * sizeof(T));
    }

    void deallocate(void* p, size_t size)
    {
        free(p);
    }


    handle_type reallocate(handle_type h, size_t size)
    {
        return (pointer) realloc(h, size);
    }


    static handle_offset_type offset(handle_type h, ptrdiff_t size)
    {
        return (uint8_t*)h + size;
    }

    size_t allocated_size(handle_type h, const allocated_size_helper& ash)
    {
        return ash.allocated_size;
    }
};

