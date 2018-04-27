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
    typedef handle_type handle_with_size;
    typedef T& handle_with_offset; // represents a pointer location past initial location of buffer
    typedef T value_type;
    typedef T* pointer;
    typedef std::size_t size_type;

protected:

    TBuffer buffer;

    single_allocator_base() {}

    single_allocator_base(const TBuffer& buffer) : buffer(buffer) {}

public:
    static CONSTEXPR handle_type invalid() { return false; }


    value_type& lock(handle_type h, int pos = 0, int count = 0)
    {
        return buffer[pos];
    }

    void unlock(handle_type h) {}

    handle_with_offset offset(handle_type h, size_t pos)
    {
        return buffer[pos];
    }

    void deallocate(handle_with_size h)
    {
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

    typedef T& handle_with_offset;

public:
    single_fixedbuf_allocator() {}

    single_fixedbuf_allocator(const TBuffer& buffer) : base_t(buffer) {}


    handle_with_size allocate_ext(size_t size)
    {
        // we can't be sure if alloc fails or succeeds with unspecified length (when TBuffer = *)
        // but since we are embedded-oriented, permit it to succeed and trust the programmer
        // took precautions
        if(len == 0)   return true;

        // remember our handle is a true/false
        return size <= len;
    }

    handle_type allocate(size_t size)
    {
        return allocate_ext(size);
    }


    handle_with_size reallocate_ext(handle_type, size_t size)
    {
        // NOTE: assuming incoming handle_type is valid

        return allocate_ext(size);
    }

    size_t size(handle_with_size h) const { return len; }

    size_t max_size() const { return len; }
};


// mainly for layer3:
// runtime (but otherwise constant) size()
// runtime (but otherwise constant) buffer*
// as before, null_terminated is merely a clue/trait for consumer class
template <class T, bool null_terminated = false>
class single_fixedbuf_runtimesize_allocator : public single_allocator_base<T, T*>
{
public:
    typedef single_allocator_base<T, T*> base_t;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::handle_type handle_type;
    typedef handle_type handle_with_size;

private:
    size_type m_buffer_size;

public:
    struct InitParam
    {
        T* buffer;
        size_type size;

        InitParam(T* buffer, size_type size) : buffer(buffer), size(size) {}
    };

    single_fixedbuf_runtimesize_allocator(const InitParam& p)
        : base_t(p.buffer), m_buffer_size(p.size)
    {}

    size_type size(handle_with_size h) const { return m_buffer_size; }

    size_type max_size() const { return m_buffer_size; }

    handle_with_size allocate_ext(size_t size)
    {
        // TODO: put in a flag only in debug mode to detect multiple alloc
        // (should only have one for single+fixed allocator)

        // remember our handle is a true/false
        return size <= max_size();
    }

    handle_type allocate(size_t size)
    {
        return allocate_ext(size);
    }


    handle_with_size reallocate_ext(handle_type, size_t size)
    {
        // NOTE: assuming incoming handle_type is valid

        return allocate_ext(size);
    }
};

// See reference implementation in dynamic_array.h
template <class TAllocator>
struct dynamic_array_helper;

template <class T, size_t len, bool null_terminated, class TBuffer>
class dynamic_array_fixedbuf_helper_base
{
    typedef single_fixedbuf_allocator<T, len, false, TBuffer> allocator_type;
    typedef ::std::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::value_type value_type;
    typedef typename allocator_type::handle_with_size handle_with_size;
    typedef typename allocator_type::handle_with_offset handle_with_offset;

    allocator_type allocator;
public:

    value_type& lock(size_type pos = 0, size_type count = 0)
    {
        return allocator.lock(true, pos, count);
    }

    void unlock() {}

    size_type capacity() const { return allocator.max_size(); }

    allocator_type& get_allocator() { return allocator; }

    handle_with_offset offset(size_type pos)
    {
        return allocator.offset(true, pos);
    }

    bool allocate(size_type sz) { return sz <= capacity(); }
    bool reallocate(size_type sz) { return sz <= capacity(); }

    bool is_allocated() const { return true; }
};

// as per https://stackoverflow.com/questions/4189945/templated-class-specialization-where-template-argument-is-a-template
// and https://stackoverflow.com/questions/49283587/templated-class-specialization-where-template-argument-is-templated-difference
// template <>
template <class T, size_t len, class TBuffer>
class dynamic_array_helper<single_fixedbuf_allocator<T, len, false, TBuffer> >
        : public dynamic_array_fixedbuf_helper_base<T, len, false, TBuffer>
{
protected:
    typedef single_fixedbuf_allocator<T, len, false, TBuffer> allocator_type;
    typedef ::std::allocator_traits<allocator_type> allocator_traits;
    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::value_type value_type;
    typedef typename allocator_type::handle_with_size handle_with_size;
    typedef typename allocator_type::handle_with_offset handle_with_offset;

    size_type m_size;

public:
    dynamic_array_helper(allocator_type*) : m_size(0) {}

    dynamic_array_helper() : m_size(0) {}

    size_type size() const { return m_size; }

    // +++ intermediate
    void size(size_type s) { m_size = s; }
    // ---
};





template <class T, size_t len, class TBuffer>
class dynamic_array_helper<single_fixedbuf_allocator<T, len, true, TBuffer> >
        //: public dynamic_array_fixedbuf_helper_base<T, len, true, TBuffer>
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
    value_type& lock(size_type pos = 0, size_type count = 0)
    {
        return allocator.lock(true, pos, count);
    }

    struct InitParam
    {
        const TBuffer& b;
        bool is_initialized;

        InitParam(const TBuffer& b, bool is_initialized = false) :
                b(b),
                is_initialized(is_initialized) {}
    };

    dynamic_array_helper(const TBuffer& b) : allocator(b)
    {
        // FIX: this won't work when we initialize to a string literal or otherwise
        // existing buffer
        allocator.lock(true) = 0;
    }

    dynamic_array_helper()
    {
        // null-terminate
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
    bool allocate(size_type sz) { return sz <= capacity(); }
    bool reallocate(size_type sz) { return sz <= capacity(); }


    // +++ intermediate
    void size(size_type s)
    {
        // FIX: Not 100% sure this is what we should do here yet
        (&allocator.lock(true))[s] = 0;
    }
    // ---

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
