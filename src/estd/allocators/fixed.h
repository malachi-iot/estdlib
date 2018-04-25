#pragma once

#include "generic.h"

namespace estd {

namespace experimental {

// Can only have its allocate function called ONCE
template <class T, size_t len>
struct single_fixedbuf_allocator
{
    typedef T value_type;
    typedef T* pointer;
    typedef bool handle_type; // really I want it an empty struct
    typedef handle_type handle_with_size;
    typedef const void* const_void_pointer;

    typedef typename nothing_allocator<T>::lock_counter lock_counter;

    // FIX: Unsure what to do about invalid in this context
    static CONSTEXPR handle_type invalid() { return false; }

    typedef T& handle_with_offset;

private:
    T buffer[len];
    size_t amount_allocated;
public:

    handle_with_size allocate_ext(size_t size)
    {
        // TODO: assert amount_allocated = 0, we can only allocate once
        // TODO: assert size <= len
        amount_allocated = size;
        return true;
    }

    handle_type allocate(size_t size)
    {
        return allocate_ext(size);
    }


    handle_with_size reallocate_ext(handle_type, size_t size)
    {
        amount_allocated += size;
        // TODO: assert size <= len
        return true;
    }

    void deallocate(handle_with_size h)
    {
        amount_allocated = 0;
    }

    value_type& lock(handle_type h) { return buffer[0]; }

    void unlock(handle_type h) {}

    size_t size(handle_with_size h) const { return amount_allocated; }

    size_t max_size() const { return len; }

    handle_with_offset offset(handle_type h, size_t pos)
    {
        return buffer[pos];
    }
};

}

}
