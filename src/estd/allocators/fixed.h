#pragma once

#include "generic.h"

namespace estd {

namespace experimental {

// Can only have its allocate function called ONCE
// maps to one and only one regular non-locking buffer
template <class T, class TBuffer>
struct single_allocator_base
{
    typedef const void* const_void_pointer;
    typedef bool handle_type; // really I want it an empty struct
    typedef T& handle_with_offset; // represents a pointer location past initial location of buffer
    typedef T value_type;
    typedef T* pointer;

protected:

    TBuffer buffer;

    single_allocator_base() {}

    single_allocator_base(const TBuffer& buffer) : buffer(buffer) {}

public:
    value_type& lock(handle_type h) { return buffer[0]; }

    void unlock(handle_type h) {}

    handle_with_offset offset(handle_type h, size_t pos)
    {
        return buffer[pos];
    }

    typedef typename nothing_allocator<T>::lock_counter lock_counter;
};

// Can only have its allocate function called ONCE
// tracks how much of the allocator has been allocated
template <class T, size_t len, class TBuffer = T[len]>
struct single_fixedbuf_allocator : public single_allocator_base<T, TBuffer>
{
    typedef single_allocator_base<T, TBuffer> base_t;

    typedef T value_type;
    typedef bool handle_type; // really I want it an empty struct
    typedef handle_type handle_with_size;

    // FIX: Unsure what to do about invalid in this context
    static CONSTEXPR handle_type invalid() { return false; }

    typedef T& handle_with_offset;

private:
    // FIX: superfluous, as technically TBuffer is always allocated to
    // 'len' size
    size_t amount_allocated;
public:
    single_fixedbuf_allocator() {}

    single_fixedbuf_allocator(const TBuffer& buffer) : base_t(buffer) {}


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

    size_t size(handle_with_size h) const { return amount_allocated; }

    size_t max_size() const { return len; }
};



// Can only have its allocate function called ONCE
// for traditional C-style null terminated strings
template <class T, size_t len, class TBuffer = T[len]>
struct single_nullterm_fixedbuf_allocator : public single_allocator_base<T, TBuffer>
{
    typedef single_allocator_base<T, TBuffer> base_t;
    typedef T value_type;
    typedef bool handle_type; // really I want it an empty struct
    typedef handle_type handle_with_size;

    // FIX: Unsure what to do about invalid in this context
    static CONSTEXPR handle_type invalid() { return false; }

    typedef T& handle_with_offset;

public:
    single_nullterm_fixedbuf_allocator() {}

    single_nullterm_fixedbuf_allocator(const TBuffer& buffer) : base_t(buffer) {}


    handle_with_size allocate_ext(size_t size)
    {
        // TODO: assert size <= len
        return true;
    }

    handle_type allocate(size_t size)
    {
        return allocate_ext(size);
    }


    handle_with_size reallocate_ext(handle_type, size_t size)
    {
        // TODO: assert size <= len
        return true;
    }

    void deallocate(handle_with_size h)
    {
    }

    size_t size(handle_with_size h) const { return ::strlen(base_t::buffer); }

    size_t max_size() const { return len; }
};


}

}
