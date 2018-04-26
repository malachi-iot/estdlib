#pragma once

#include "generic.h"
#include <string.h> // for strlen

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
// null_terminated flag mainly serves as a trait/clue to specializations
template <class T, size_t len, bool null_terminated = false, class TBuffer = T[len]>
struct single_fixedbuf_allocator : public single_allocator_base<T, TBuffer>
{
    typedef single_allocator_base<T, TBuffer> base_t;

    typedef T value_type;
    typedef bool handle_type; // really I want it an empty struct
    typedef handle_type handle_with_size;

    // FIX: Unsure what to do about invalid in this context
    static CONSTEXPR handle_type invalid() { return false; }

    typedef T& handle_with_offset;

public:
    single_fixedbuf_allocator() {}

    single_fixedbuf_allocator(const TBuffer& buffer) : base_t(buffer) {}


    handle_with_size allocate_ext(size_t size)
    {
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

    size_t size(handle_with_size h) const { return len; }

    size_t max_size() const { return len; }
};

// See reference implementation in dynamic_array.h
template <class TAllocator>
struct dynamic_array_helper;

// as per https://stackoverflow.com/questions/4189945/templated-class-specialization-where-template-argument-is-a-template
// and https://stackoverflow.com/questions/49283587/templated-class-specialization-where-template-argument-is-templated-difference
// template <>
template <class T, size_t len, class TBuffer>
class dynamic_array_helper<single_fixedbuf_allocator<T, len, false, TBuffer> >
{
protected:
    typedef single_fixedbuf_allocator<T, len, false, TBuffer> allocator_type;
    typedef ::std::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::value_type value_type;
    typedef typename allocator_type::handle_with_size handle_with_size;
    typedef typename allocator_type::handle_with_offset handle_with_offset;

    allocator_type allocator;

    size_type m_size;

public:
    dynamic_array_helper(allocator_type*) : m_size(0) {}

    size_type capacity() const { return allocator.max_size(); }
    size_type size() const { return m_size; }

    // +++ intermediate
    void size(size_type s) { m_size = s; }
    // ---

    value_type& lock() { return allocator.lock(true); }
    void unlock() {}

    allocator_type& get_allocator() { return allocator; }

    handle_with_offset offset(size_type pos)
    {
        return allocator.offset(true, pos);
    }

    // TODO: ensure sz doesn't exceed len
    void allocate(size_type sz) {}
    void reallocate(size_type sz) {}


    bool is_allocated() const { return true; }
};





template <class T, size_t len, class TBuffer>
class dynamic_array_helper<single_fixedbuf_allocator<T, len, true, TBuffer> >
{
protected:
    typedef single_fixedbuf_allocator<T, len, true, TBuffer> allocator_type;
    typedef ::std::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_type::handle_with_size handle_with_size;
    typedef typename allocator_type::handle_with_offset handle_with_offset;
    typedef T value_type;

    // fixed length buffer in which a null terminated string is/shall be located
    allocator_type allocator;

public:
    // TODO: Need to improve parameter passing in for initialization of allocator_type
    dynamic_array_helper(allocator_type*)
    {
        // auto init null-termination
        // normally we leave buffers untouched but dynamic array it's by design
        // we start with 0 'utilized'
        allocator.lock(true) = 0;
    }

    size_type capacity() const { return allocator.max_size(); }

    size_type size() const
    {
        // FIX: Make this work with any value_type, not just char, since
        // dynamic_array can be utilized not just with string
        // FIX: Do away with this nasty const-forcing.  It's an artifact of our
        // underlying nature of lock/unlock having side effects but necessary
        // to get at what is normally expected to be an unchanging pointer location
        allocator_type& nonconst_a = const_cast<allocator_type&>(allocator);
        return ::strlen(&nonconst_a.lock(true));
    }

    // TODO: ensure sz doesn't exceed len
    void allocate(size_type sz) {}
    void reallocate(size_type sz) {}


    // +++ intermediate
    void size(size_type s)
    {
        // FIX: Not 100% sure this is what we should do here yet
        (&allocator.lock(true))[s] = 0;
    }
    // ---

    value_type& lock() { return allocator.lock(true); }
    void unlock() {}


    allocator_type& get_allocator() const { return allocator; }

    handle_with_offset offset(size_type pos)
    {
        return allocator.offset(true, pos);
    }

    bool is_allocated() const { return true; }
};

}

}
