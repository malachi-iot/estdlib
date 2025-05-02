#pragma once

#include "generic.h"
#include "../traits/allocator_traits.h"
#include "../internal/container/accessor.h"
#include "../internal/container/traditional_accessor.h"
#include "../internal/container/iterator.h"
#include "../internal/array.h"
#include <string.h> // for strlen
#include "../allocators/handle_desc.h"
#include "../initializer_list.h"
#include "../limits.h"
#ifdef FEATURE_CPP_STATIC_ASSERT
// DEBT: Change over to cassert - first have to see if AVR permits that
#include <assert.h>
#endif

#include "../internal/macro/push.h"

namespace estd {

namespace internal {


// This and specializations are experimental at the moment
template <class Buffer>
struct allocator_buffer_traits
{
    typedef bool handle_type;
};


template <class T>
struct allocator_buffer_traits<T*>
{
    typedef T* handle_type;
};

template <class T>
struct allocator_buffer_traits<T[]>
{
    using handle_type = T*;
};

template <class Base>
struct allocator_buffer_traits<array_base2<Base>>
{
    using handle_type = typename array_base2<Base>::pointer;
};


/// Core of allocators which can only perform one allocation on one singular, contiguous buffer
// Can only have its allocate function called ONCE
// maps to one and only one regular non-locking buffer
// also this is a stateful allocator, by nature of TBuffer taking up some space
// if one wishes to be stateless, a different (base) class should be used
template <class T, class Buffer, typename Size,
    typename Diff = estd::make_signed_t<Size>>
struct single_allocator_base
{
    // TODO: Phase this out in favor of SFINAE/detection method from allocator_traits
    typedef const void* const_void_pointer;
    // really I want it an empty struct.  Bool is somewhat convenient though since we can
    // represent an invalid handle too
#ifdef FEATURE_ESTD_ENHANCED_HANDLE_EXP
    typedef single_allocator_handle handle_type;
#else
    typedef bool handle_type;
#endif
    using size_type = Size;
    using difference_type = Diff;

    typedef allocator_buffer_traits<Buffer> buffer_traits; // EXPERIMENTAL

    //typedef handle_type handle_with_size;
    typedef estd::internal::handle_with_only_offset<handle_type, size_type> handle_with_offset;
    typedef T value_type;
    // NOTE: these pointer typedefs are actually deprecated, but useful for use to have
    // so I'm keeping them around
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef estd::experimental::stateful_nonlocking_accessor<single_allocator_base> accessor;

    // For scenarios where someone wishes to specifically treat this allocated
    // item as a container of T
    using iterator = locking_iterator<
        single_allocator_base,
        traditional_accessor<value_type> >;

protected:
    // NOTE: Would make private but 'adjust_offset_exp' (which basic_string_view uses)
    // needs it.
    Buffer buffer;

protected:

    // use this instead of direct buffer reference, for upcoming aligned_storage
    // compatibility
    ESTD_CPP_CONSTEXPR(17) pointer data(size_type offset = 0)
    {
        return static_cast<pointer>(&buffer[offset]);
    }

    constexpr const_pointer data(size_type offset = 0) const
    {
        return static_cast<const_pointer>(&buffer[offset]);
    }

    ESTD_CPP_DEFAULT_CTOR(single_allocator_base)

#if __cpp_variadic_templates
    template <typename ...T2>
    constexpr explicit single_allocator_base(estd::in_place_t, T2&&...ts) :
        buffer{std::forward<T2>(ts)...}
    {

    }
#endif

    ESTD_CPP_CONSTEXPR_RET EXPLICIT single_allocator_base(const Buffer& buffer) : buffer(buffer) {}

public:
    static CONSTEXPR handle_type invalid() { return false; }
    // 'valid()' handle call only applicable for singlular-allocator scenarios
    static CONSTEXPR handle_type valid() { return true; }

    // technically we ARE locking since we have to convert the dummy 'bool' handle
    // to a pointer
    // FIX: need to make two different tags so that we can differenciate between
    // handles which need a conversion and handles whose memory may move around

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

    ESTD_CPP_CONSTEXPR(17) value_type& lock(handle_type, int pos = 0, int = 0)
    {
        return *data(pos);
    }

    ESTD_CPP_CONSTEXPR(17) value_type& lock(const handle_with_offset& h, int pos = 0, int = 0)
    {
        return *data(h.offset() + pos);
    }

    constexpr const value_type& clock(handle_type, int pos = 0, int = 0) const
    {
        return *data(pos);
    }

    constexpr const value_type& clock(const handle_with_offset& h, int pos = 0, int = 0) const
    {
        return *data(h.offset() + pos);
    }

    ESTD_CPP_CONSTEXPR(17) void unlock(handle_type) {}

    ESTD_CPP_CONSTEXPR(17) void cunlock(handle_type) const {}

    handle_with_offset offset(handle_type h, size_t pos) const
    {
        return handle_with_offset(h, pos);
    }

    /*
    void deallocate(handle_with_size h, size_type count)
    {
    } */

    handle_type reallocate(handle_type h, size_t)
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
// compile-time size()
// Loosely corresponds to layer1/layer2 behavior
// len can == 0 in which case we're in unbounded mode
// NOTE: uninitialized_array is handy to use for TBuffer to avoid value-init of all the elements
template <
        class T, size_t len, class Buffer = T[len],
        class Size = typename internal::deduce_fixed_size_t<len>::size_type>
struct single_fixedbuf_allocator : public
        single_allocator_base<T, Buffer, Size>
{
    using base_type = single_allocator_base<T, Buffer, Size>;
    using typename base_type::size_type;

    typedef typename base_type::value_type value_type;
    typedef bool handle_type; // really I want it an empty struct, though now code expects a bool
    typedef handle_type handle_with_size;

public:
    // experimental tag reflecting that this memory block will never move
    typedef void is_pinned_tag_exp;

    ESTD_CPP_DEFAULT_CTOR(single_fixedbuf_allocator)

#if __cpp_variadic_templates
    template <class ...T2>
    constexpr explicit single_fixedbuf_allocator(in_place_t, T2&&...ts) :
        base_type(in_place_t{}, std::forward<T2>(ts)...)
    {

    }
#endif

    // FIX: something bizzare is happening here and base_t is ending
    // up as map_base during debug session
    ESTD_CPP_CONSTEXPR_RET EXPLICIT single_fixedbuf_allocator(const Buffer& buffer) :
        base_type(buffer) {}


    handle_with_size allocate_ext(size_t size) const
    {
        // we can't be sure if alloc fails or succeeds with unspecified length (when TBuffer = *)
        // but since we are embedded-oriented, permit it to succeed and trust the programmer
        // took precautions
        if(len == 0)   return true;

        // remember our handle is a true/false
        return size <= len;
    }

    // TODO: Make a debug mode which actually does track an allocated handle, just to make
    // sure we don't allocate a 2nd time for these fixed+singular scenarios
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
    static ESTD_CPP_CONSTEXPR_RET size_t max_size()
    {
        return len == 0 ? estd::numeric_limits<size_t>::max() : len;
    }

    size_t size(handle_with_size) const { return max_size(); }
};


// mainly for layer3:
// runtime (but otherwise constant) size()
// runtime (but otherwise constant) buffer*
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

    ESTD_CPP_CONSTEXPR_RET EXPLICIT single_fixedbuf_runtimesize_allocator(T* buffer, size_type size) :
        base_t(buffer),
        m_buffer_size(size)
    {

    }

public:
    struct InitParam
    {
        T* buffer;      // incoming layer3 buffer
        size_type size; // size of layer3 buffer (remember this represents allocated size)

        ESTD_CPP_CONSTEXPR_RET InitParam(T* buffer, size_type size) : buffer(buffer), size(size) {}
    };

    ESTD_CPP_CONSTEXPR_RET EXPLICIT single_fixedbuf_runtimesize_allocator(const InitParam& p)
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

    handle_with_size allocate_ext(size_type size)
    {
        // TODO: put in a flag only in debug mode to detect multiple alloc
        // (should only have one for single+fixed allocator)

        // remember our handle is a true/false
        return size <= max_size();
    }

    handle_type allocate(size_type size)
    {
        return allocate_ext(size);
    }


    handle_with_size reallocate_ext(handle_type, size_type size)
    {
        // NOTE: assuming incoming handle_type is valid

        return allocate_ext(size);
    }
};

}


namespace layer1 {

// Fixed in place singular buffer
template <class T, size_t len,
    class Array = conditional_t<
        is_integral<T>::value,
        T[len],
        internal::uninitialized_array<T, len>>>
#if __cpp_alias_templates
using allocator = estd::internal::single_fixedbuf_allocator<T, len, Array>;

namespace legacy {
#endif

// DEBT: See allocator-test.cpp, for specialization (in that one case) something about the aliased
// version malfunctions
template <class T, size_t len>
struct allocator : estd::internal::single_fixedbuf_allocator<T, len,
    estd::internal::uninitialized_array<T, len> >
{

};

#if __cpp_alias_templates
}
#endif

}

namespace layer2 {

template <class T, size_t len>
struct allocator : estd::internal::single_fixedbuf_allocator<T, len, T*>
{
    typedef estd::internal::single_fixedbuf_allocator<T, len, T*> base_t;

    ESTD_CPP_CONSTEXPR_RET EXPLICIT allocator(T* buf) : base_t(buf) {}

    // DEBT: Compare len to N and make sure we don't get ourselves into trouble
    template <size_t N>
    ESTD_CPP_CONSTEXPR_RET EXPLICIT allocator(T (&array) [N]) : base_t(array, N)
    {

    }
};

}

namespace layer3 {

// malleable is a special edge case flag which indicates that initial buffer* can
// move and size can be adjusted.  Generally speaking you don't want this, but
// special cases like basic_string_view benefit from this adjustability
template <class T, class Size = std::size_t, bool malleable = false>
struct allocator : estd::internal::single_fixedbuf_runtimesize_allocator<T, Size>
{
    typedef estd::internal::single_fixedbuf_runtimesize_allocator<T, Size> base_t;
    typedef base_t base_type;
    typedef typename base_t::handle_type handle_type;
    typedef typename base_t::handle_with_offset handle_with_offset;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::difference_type difference_type;

#ifdef __cpp_initializer_lists
    constexpr allocator(std::initializer_list<T> initlist) : base_type(initlist)
    {
    }
#endif

    template <Size N>
    ESTD_CPP_CONSTEXPR_RET EXPLICIT allocator(T (&array) [N]) : base_type(array, N)
    {

    }

    ESTD_CPP_FORWARDING_CTOR(allocator)

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
        : estd::internal::handle_descriptor_base<
                internal::single_fixedbuf_allocator<T, N, TBuffer, TSize>,
                true,
                true,
                true,
                true>

{
    typedef estd::internal::handle_descriptor_base<
            estd::internal::single_fixedbuf_allocator<T, N, TBuffer, TSize>,
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

#include "../internal/macro/pop.h"
