#pragma once

#include "generic.h"
#include "../traits/allocator_traits.h"
#include "../internal/handle_with_offset.h"
#include <string.h> // for strlen
#include "../allocators/handle_desc.h"

namespace estd {

namespace internal {

// Can only have its allocate function called ONCE
// maps to one and only one regular non-locking buffer
// also this is a stateful allocator, by nature of TBuffer taking up some space
// if one wishes to be stateless, a different (base) class should be used
template <class T, class TBuffer, typename TSize>
struct single_allocator_base
{
    typedef const void* const_void_pointer;
    // really I want it an empty struct.  Bool is somewhat convenient though since we can
    // represent an invalid handle too
    typedef bool handle_type;
    typedef TSize size_type;
    typedef handle_type handle_with_size;
    //typedef T& handle_with_offset; // represents a pointer location past initial location of buffer
    typedef estd::internal::handle_with_only_offset<handle_type, size_type> handle_with_offset;
    typedef T value_type;
    typedef T* pointer;
    typedef estd::experimental::stateful_nonlocking_accessor<single_allocator_base> accessor;

protected:

    TBuffer buffer;

    // for inline buffers, we want the option of leaving it untouched
    single_allocator_base() {}

    single_allocator_base(const TBuffer& buffer) : buffer(buffer) {}

public:
    static CONSTEXPR handle_type invalid() { return false; }

    // technically we ARE locking since we have to convert the dummy 'bool' handle
    // to a pointer
    static CONSTEXPR bool is_locking() { return true; }

    static CONSTEXPR bool is_stateful() { return true; }

    static CONSTEXPR bool is_singular() { return true; }

    static CONSTEXPR bool has_size() { return true; }


    value_type& lock(handle_type h, int pos = 0, int count = 0)
    {
        return buffer[pos];
    }

    value_type& lock(const handle_with_offset& h, int pos = 0, int count = 0)
    {
        return buffer[h.offset() + pos];
    }

    const value_type& clock(handle_type h, int pos = 0, int count = 0) const
    {
        return buffer[pos];
    }

    const value_type& clock(const handle_with_offset& h, int pos = 0, int count = 0) const
    {
        return buffer[h.offset() + pos];
    }

    void unlock(handle_type h) {}

    void cunlock(handle_type h) const {}

    handle_with_offset offset(handle_type h, size_t pos) const
    {
        return handle_with_offset(h, pos);
    }

    void deallocate(handle_with_size h, size_type count)
    {
    }

    handle_type reallocate(handle_type h, size_t len)
    {
        // Not supported operation
        assert(false);

        return h;
    }


    typedef typename nothing_allocator<T>::lock_counter lock_counter;
};

// Can only have its allocate function called ONCE
// tracks how much of the allocator has been allocated
// null_terminated flag mainly serves as a trait/clue to specializations
// len can == 0 in which case we're in unbounded mode
template <
        class T, size_t len, bool null_terminated = false, class TBuffer = T[len],
        class TSize = typename internal::deduce_fixed_size_t<len>::size_type>
struct single_fixedbuf_allocator : public
        single_allocator_base<T, TBuffer, TSize>
{
    typedef single_allocator_base<T, TBuffer, TSize> base_t;

    typedef T value_type;
    typedef bool handle_type; // really I want it an empty struct
    typedef handle_type handle_with_size;
    typedef typename base_t::size_type size_type;

public:
    single_fixedbuf_allocator() {}

    // FIX: something bizzare is happening here and base_t is ending
    // up as map_base during debug session
    single_fixedbuf_allocator(const TBuffer& buffer) : base_t(buffer) {}


    handle_type reallocate(handle_type, size_type size)
    {
        return size <= len;
    }

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

    // FIX: returns actual maximum size of unsigned,not ideal
    size_t max_size() const { return len == 0 ? -1 : len; }

    size_t size(handle_with_size h) const { return max_size(); }
};


// mainly for layer3:
// runtime (but otherwise constant) size()
// runtime (but otherwise constant) buffer*
// as before, null_terminated is merely a clue/trait for consumer class
template <class T, bool null_terminated = false, class TSize = std::size_t>
class single_fixedbuf_runtimesize_allocator : public single_allocator_base<T, T*, TSize>
{
public:
    typedef single_allocator_base<T, T*, TSize> base_t;
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

    // FIX: This does *NOT* belong here, only a helper should do this
    // null_teriminated flag in the allocator is merely a trait
    size_type max_size() const
    {
        return m_buffer_size;
        //return m_buffer_size - (null_terminated ? 1 : 0);
    }

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

// TODO: Fixup name
template <class TAllocator, bool null_terminated>
struct length_helper;

// null terminated
template <class TAllocator>
struct length_helper<TAllocator, true>
{
    typedef TAllocator allocator_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::handle_type handle_type;

    bool empty(const allocator_type& a, const handle_type& h) const
    {
        const value_type* v = &a.clock(h, 0, 1);

        bool is_terminator = *v == 0;

        a.cunlock(h);

        return is_terminator;
    }

    size_type size(const TAllocator& a, const handle_type& h) const
    {
#ifdef FEATURE_CPP_STATIC_ASSERT
        // specialization required if we aren't null terminated (to track size variable)
        //static_assert(null_terminated, "Utilizing this size method requires null termination = true");
#endif

        const value_type* s = &a.clock(h);

        // FIX: use char_traits string length instead
        size_type sz = strlen(s);

        a.cunlock(h);

        return sz;
    }

    // +++ temporary
    // semi-brute forces size by stuffing a null terminator at the specified spot
    void size(TAllocator& a, const handle_type& h, size_type len)
    {
        /*
        if(len > base_t::capacity())
        {
            // FIX: issue some kind of warning
        } */

        a.lock(h, len, 1) = 0;
        a.unlock(h);
    }
    // ---
};


// explicitly sized
template <class TAllocator>
struct length_helper<TAllocator, false>
{
    typedef TAllocator allocator_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::handle_type handle_type;

protected:
    size_type m_size;

    length_helper() :
            m_size(0)
    {}


public:

    bool empty(const allocator_type& a, const handle_type& h) const
    {
        return m_size == 0;
    }

    size_type size(const TAllocator& a, const handle_type& h) const
    {
        return m_size;
    }

    // +++ temporary
    void size(TAllocator& a, const handle_type& h, size_type len)
    {
        m_size = len;
    }
    // ---
};

// ---


// intermediate class as we transition to handle_descriptor.  Eventually phase this out
template <class TAllocator, bool null_terminated>
class handle_descriptor_helper :
        public handle_descriptor<TAllocator>,
        length_helper<TAllocator, null_terminated>
{
    typedef handle_descriptor<TAllocator> base_t;
    typedef length_helper<TAllocator, null_terminated> length_helper_t;

public:
    handle_descriptor_helper() {}

    template <class TAllocatorParameter>
    handle_descriptor_helper(TAllocatorParameter& p) : base_t(p) {}


    static CONSTEXPR bool uses_termination() { return null_terminated; }

    typedef typename base_t::allocator_type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::handle_with_offset handle_with_offset;

    // account for null-termination during a max_size request
    size_type max_size() const
    {
        return base_t::get_allocator().max_size() - (null_terminated ? 1 : 0);
    }


    // repurposing/renaming of what size meant before (ALLOCATED) vs now
    // (USED within ALLOCATED)
    size_type capacity() const { return base_t::size(); }


    // Helper for old dynamic_array code.  New one I'm thinking caller
    // can shoulder this burden
    handle_with_offset offset(size_type pos) const
    {
        return base_t::get_allocator().offset(base_t::handle(), pos);
    }

    // remember, dynamic_array_helper size() refers not to ALLOCATED size, but rather
    // 'used' size within that allocation.  For this variety, we are null terminated
    size_type size() const
    {
        if(!base_t::is_allocated()) return 0;

        return length_helper_t::size(base_t::get_allocator(), base_t::handle());
    }

    void size(size_type n)
    {
        length_helper_t::size(base_t::get_allocator(), base_t::handle(), n);
    }
};


// applies generally to T[N], RW buffer but also to non-const T*
// applies specifically to null-terminated
template <class T, size_t len, class TBuffer>
class dynamic_array_helper<single_fixedbuf_allocator<T, len, true, TBuffer> >
        : public handle_descriptor_helper<single_fixedbuf_allocator<T, len, true, TBuffer>, true >
{
    typedef handle_descriptor_helper<single_fixedbuf_allocator<T, len, true, TBuffer>, true > base_t;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::handle_type handle_type;

    // FIX: Iron out exactly where we really assign size(0) - sometimes we
    // want to pre-initialize our buffer so size(0) is not an always thing
    dynamic_array_helper(TBuffer& b) : base_t(b)
    {
        base_t::size(0);
    }

    dynamic_array_helper(const TBuffer& b) : base_t(b)
    {
        // FIX: This only works for NULL-terminated scenarios
        // we still need to assign a length of 0 for explicit lenght scenarios
    }

    dynamic_array_helper()
    {
        base_t::size(0);
    }
};





// for basic_string_view and const_string
// runtime size information is stored in allocator itself, not helper
// (not null terminated, since it's runtime-const fixed size)
template <class T>
class dynamic_array_helper<single_fixedbuf_runtimesize_allocator<const T, false, size_t> >
        : public handle_descriptor_parity<single_fixedbuf_runtimesize_allocator<const T, false, size_t>, true, true>
{
    typedef handle_descriptor_parity<single_fixedbuf_runtimesize_allocator<const T, false, size_t>, true, true> base_t;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::value_type value_type;

    // repurposing/renaming of what size meant before (ALLOCATED) vs now
    // (USED within ALLOCATED)
    size_type capacity() const { return base_t::size(); }

    size_type max_size() const { return capacity(); }

    template <class TParam>
    dynamic_array_helper(const TParam& p) :
        base_t(p) {}

    dynamic_array_helper(const dynamic_array_helper& copy_from) :
        base_t(copy_from.get_allocator())
    {
    }
};



// runtime (layer3-ish) version
template <class T, bool null_terminated>
class dynamic_array_helper<single_fixedbuf_runtimesize_allocator<T, null_terminated> > :
        public handle_descriptor_helper<single_fixedbuf_runtimesize_allocator<T, null_terminated>, null_terminated >
{
    typedef handle_descriptor_helper<single_fixedbuf_runtimesize_allocator<T, null_terminated>, null_terminated > base_t;
    typedef typename base_t::size_type size_type;

public:
    template <class TInitParam>
    dynamic_array_helper(const TInitParam& p) : base_t(p) {}
};

}

}
