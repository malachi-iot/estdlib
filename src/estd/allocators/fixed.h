#pragma once

#include "generic.h"
#include "../traits/allocator_traits.h"
#include "../internal/handle_with_offset.h"
#include <string.h> // for strlen
#include "../allocators/handle_desc.h"
#include "../initializer_list.h"
#ifdef FEATURE_CPP_STATIC_ASSERT
#include <assert.h>
#endif

namespace estd {

namespace internal {

// Can only have its allocate function called ONCE
// maps to one and only one regular non-locking buffer
// also this is a stateful allocator, by nature of TBuffer taking up some space
// if one wishes to be stateless, a different (base) class should be used
template <class T, class TBuffer, typename TSize,
          typename TDiff = typename estd::make_signed<TSize>::type>
struct single_allocator_base
{
    typedef const void* const_void_pointer;
    // really I want it an empty struct.  Bool is somewhat convenient though since we can
    // represent an invalid handle too
    typedef bool handle_type;
    typedef TSize size_type;
    typedef TDiff difference_type;
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

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static CONSTEXPR bool is_locking() { return true; }

    static CONSTEXPR bool is_stateful() { return true; }

    static CONSTEXPR bool is_singular() { return true; }

    static CONSTEXPR bool has_size() { return true; }
#endif

    typedef void is_locking_tag;
    typedef void has_size_tag;
    typedef void is_singular_tag;
    typedef void is_stateful_tag;

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
#ifdef FEATURE_CPP_STATIC_ASSERT
        // Not supported operation
        assert(false);
#endif

        return h;
    }


#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
    typedef typename nothing_allocator<T>::lock_counter lock_counter;
#endif
};

// Can only have its allocate function called ONCE
// tracks how much of the allocator has been allocated
// null_terminated flag mainly serves as a trait/clue to specializations
// len can == 0 in which case we're in unbounded mode
template <
        class T, size_t len, class TBuffer = T[len],
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
template <class T, class TSize = std::size_t>
class single_fixedbuf_runtimesize_allocator : public single_allocator_base<T, T*, TSize>
{
public:
    typedef single_allocator_base<T, T*, TSize> base_t;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::handle_type handle_type;
    typedef handle_type handle_with_size;

private:
    // TODO: Consider consolidating with the handle_descriptor crew
    size_type m_buffer_size;

protected:
    // primarily used with special use case like basic_string_view which needs to adjust
    // 'allocated' size manually during remove_suffix
    void set_size(size_type size)
    {
        m_buffer_size = size;
    }

    explicit single_fixedbuf_runtimesize_allocator(T* buffer, size_type size) :
        base_t(buffer),
        m_buffer_size(size)
    {

    }

public:
    struct InitParam
    {
        T* buffer;      // incoming layer3 buffer
        size_type size; // size of layer3 buffer (remember this represents allocated size)

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

}

namespace layer1 {

template <class T, size_t len>
struct allocator : internal::single_fixedbuf_allocator<T, len>
{

};

}

namespace layer2 {

template <class T, size_t len>
struct allocator : internal::single_fixedbuf_allocator<T, len, T*>
{
    typedef internal::single_fixedbuf_allocator<T, len, T*> base_t;

    allocator(T* buf) : base_t(buf) {}

    // TODO: Need to do that SFINAE trick to force this to invoke
    template <size_t N>
    allocator(T (&array) [N]) : base_t(array, N)
    {

    }
};

}

namespace layer3 {

// malleable is a special edge case flag which indicates that initial buffer* can
// move and size can be adjusted.  Generally speaking you don't want this, but
// special cases like basic_string_view benefit from this adjustability
template <class T, class TSize = std::size_t, bool malleable = false>
struct allocator : internal::single_fixedbuf_runtimesize_allocator<T, TSize>
{
    typedef internal::single_fixedbuf_runtimesize_allocator<T, TSize> base_t;
    typedef typename base_t::handle_type handle_type;
    typedef typename base_t::handle_with_offset handle_with_offset;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::difference_type difference_type;

#ifdef FEATURE_CPP_INITIALIZER_LIST
    allocator(std::initializer_list<T> initlist) : base_t(initlist)
    {
    }
#endif

    template <TSize N>
    allocator(T (&array) [N]) : base_t(array, N)
    {

    }

    template <class TAllocatorParam>
    allocator(const TAllocatorParam& p) : base_t(p) {}

    // Experimental - malleable allocator.  Used primarily for basic_string_view
    // remove_prefix and remove_suffix
    // TODO: turn these API on or off depending on malleable flag

    // can adjust positively or negatively (operates like pointer math and/or handle_with_offset)
    void adjust_offset_exp(handle_type h, difference_type offset)
    {
        base_t::buffer += offset;
        base_t::set_size(base_t::size(h) - offset);
    }

    void set_size_exp(handle_type h, size_type size)
    {
        base_t::set_size(size);
    }
};

}

// FIX: Very nasty explicit specializations for handle_descriptor on various canned fixed allocators
//      plan to revise this using either typedef-tags or CONSTEXPR bool (not bool functions) so that
//      we can unify c++11 and pre-c++11 approach
// when FEATURE_ESTD_STRICT_DYNAMIC_ARRAY is in play, these are not necessary
#if !defined(FEATURE_CPP_CONSTEXPR) && !defined(FEATURE_ESTD_STRICT_DYNAMIC_ARRAY)
template <class T, size_t N, class TBuffer, class TSize, class TTraits>
struct handle_descriptor<internal::single_fixedbuf_allocator<T, N, TBuffer, TSize>, TTraits >
        : internal::handle_descriptor_base<
                internal::single_fixedbuf_allocator<T, N, TBuffer, TSize>,
                true,
                true,
                true,
                true>

{
    typedef internal::handle_descriptor_base<
            internal::single_fixedbuf_allocator<T, N, TBuffer, TSize>,
            true,
            true,
            true,
            true> base_t;

    handle_descriptor() : base_t(true) {}

    template <class TAllocatorParam>
    handle_descriptor(const TAllocatorParam& p) : base_t(p, true) {}
};


template <class T, size_t N, class TTraits>
struct handle_descriptor<layer1::allocator<T, N>, TTraits >
        : internal::handle_descriptor_base<
                layer1::allocator<T, N>,
                true,
                true,
                true,
                true>

{
    typedef internal::handle_descriptor_base<
            layer1::allocator<T, N>,
            true,
            true,
            true,
            true> base_t;

    handle_descriptor() : base_t(true) {}

    template <class TAllocatorParam>
    handle_descriptor(const TAllocatorParam& p) : base_t(p, true) {}
};


template <class T, class TSize, class TTraits>
struct handle_descriptor<internal::single_fixedbuf_runtimesize_allocator<T, TSize>, TTraits >
        : internal::handle_descriptor_base<
                internal::single_fixedbuf_runtimesize_allocator<T, TSize>,
                true,
                true,
                true,
                true>

{
    typedef internal::handle_descriptor_base<
            internal::single_fixedbuf_runtimesize_allocator<T, TSize>,
            true,
            true,
            true,
            true> base_t;

    handle_descriptor() : base_t(true) {}

    template <class TAllocatorParam>
    handle_descriptor(const TAllocatorParam& p) :
//            base_t(p, typename TTraits::invalid_handle()) {}
            base_t(p, true) {}
};

template <class T, class TSize, class TTraits>
struct handle_descriptor<layer3::allocator<T, TSize>, TTraits >
        : internal::handle_descriptor_base<
                layer3::allocator<T, TSize>,
                true,
                true,
                true,
                true>

{
    typedef internal::handle_descriptor_base<
            layer3::allocator<T, TSize>,
            true,
            true,
            true,
            true> base_t;

    handle_descriptor() : base_t(true) {}

    template <class TAllocatorParam>
    handle_descriptor(const TAllocatorParam& p) :
//            base_t(p, typename TTraits::invalid_handle()) {}
            base_t(p, true) {}
};
#endif


}
