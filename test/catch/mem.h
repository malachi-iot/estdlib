#pragma once

#include "estd/memory.h"
#include <cstddef> // for ptrdiff_t

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

    typedef T value_type;
    typedef value_type* pointer;
    typedef const void* const_void_pointer;
    typedef value_type* handle_type;

    // primarily for subscript/array operations
    class handle_with_offset
    {
        pointer loc;

    public:
        handle_with_offset(pointer loc) : loc(loc) {}

        T& lock() { return *loc; }

        void unlock() {}

        // prefix version
        handle_with_offset& operator++()
        {
            loc++;
            return *this;
        }


        handle_with_offset& operator--()
        {
            loc--;
            return *this;
        }

        bool operator==(const handle_with_offset& compare_to) const
        {
            return loc == compare_to.loc;
        }

        bool operator!=(const handle_with_offset& compare_to) const
        {
            return loc != compare_to.loc;
        }

        handle_with_offset& operator=(const T& value)
        {
            // will autolock and unlock if necessary
            *loc = value;
        }

        operator T&()
        {
            // will autolock and unlock if necessary, returning only a const copy
            return *loc;
        }
    };

    static handle_type invalid() { return NULLPTR; }

    class handle_with_size
    {
        handle_type h;
        size_t m_size;

    public:
        // FIX: Having a 0 size should never be valid, but we have scenarios where
        // we need to track an invalid handle type (unallocated)
        handle_with_size(handle_type h, size_t size = 0) :
            h(h), m_size(size) {}

        size_t size() const { return m_size; }

        operator handle_type() const { return h; }
    };

    T& lock(handle_type h, size_t pos = 0, size_t count = 0) { return *(h + pos); }

    const T& clock_experimental(handle_type h, size_t pos = 0, size_t count = 0) { return *(h + pos); }

    void unlock(handle_type) { }

    pointer allocate(size_t size)
    {
        return (pointer) malloc(size * sizeof(T));
    }

    // consider merging this with regular allocate and letting optimizer shake out
    // pointer access
    handle_with_size allocate_ext(size_t size)
    {
        return handle_with_size(allocate(size), size);
    }


    size_t size(const handle_with_size& h) const
    {
        return h.size();
    }


    void deallocate(handle_type p, size_t size)
    {
        free(p);
    }

    void deallocate(handle_with_size h)
    {
        free(h);
    }


    handle_type reallocate(handle_type h, size_t size)
    {
        return (pointer) realloc(h, size);
    }


    handle_with_size reallocate_ext(handle_type h, size_t size)
    {
        return handle_with_size(reallocate(h, size), size);
    }


    // TODO: template this to work with both handle_with_size or not
    handle_with_offset offset(handle_type h, ptrdiff_t size)
    {
        return handle_with_offset(h + size);
    }
};

