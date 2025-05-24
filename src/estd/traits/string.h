#pragma once

#include "../internal/platform.h"
#include "../policy/base.h"
#include "../internal/fwd/string.h"

namespace estd {

// TODO: Gonna need stand-in std::char_traits for Arduino,
//       -or- commit completely to our very own estd::char_traits
/*
template<class CharT> struct char_traits;

template<> struct char_traits<char>
{
    typedef char char_type;
    typedef int int_type;
    typedef char nonconst_char_type;
};


template<> struct char_traits<const char>
{
    typedef const char char_type;
    typedef int int_type;
    typedef char nonconst_char_type;
}; */


namespace experimental {

template <bool constant>
struct is_const_tag_exp_base {};

template <>
struct is_const_tag_exp_base<true> { typedef void is_constant_tag_exp; };

}

// TODO: Consider moving this out into an impl/string.h
namespace internal { namespace impl {

// DEBT: 30JUL23 Pretty sure this crusty old code won't compile now
#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY

// applies generally to T[N], RW buffer but also to non-const T*
// applies specifically to null-terminated
template <class T, size_t len, class TBuffer, class CharTraits, bool is_const>
class dynamic_array<
        single_fixedbuf_allocator<T, len, TBuffer>,
        experimental::null_terminated_string_policy<CharTraits, int16_t, is_const> >
        : public dynamic_array_base<single_fixedbuf_allocator<T, len, TBuffer>, true, false >
{
    typedef dynamic_array_base<single_fixedbuf_allocator<T, len, TBuffer>, true, false > base_t;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::handle_type handle_type;

    // FIX: Iron out exactly where we really assign size(0) - sometimes we
    // want to pre-initialize our buffer so size(0) is not an always thing
    dynamic_array(TBuffer& b) : base_t(b)
    {
        base_t::size(0);
    }

    dynamic_array(const TBuffer& b) : base_t(b)
    {
        // FIX: This only works for NULL-terminated scenarios
        // we still need to assign a length of 0 for explicit lenght scenarios
    }

    dynamic_array()
    {
        base_t::size(0);
    }
};


// runtime (layer3-ish) version
// null terminated
template <class T, class CharTraits, bool is_const>
class dynamic_array<
        single_fixedbuf_runtimesize_allocator<T>,
        experimental::null_terminated_string_policy<CharTraits, int16_t, is_const> > :
        public dynamic_array_base<single_fixedbuf_runtimesize_allocator<T>, true, false >
{
    typedef dynamic_array_base<single_fixedbuf_runtimesize_allocator<T>, true, false > base_t;
    typedef typename base_t::size_type size_type;

public:
    template <class TInitParam>
    dynamic_array(const TInitParam& p) : base_t(p) {}
};



// for basic_string_view and const_string
// runtime size information is stored in allocator itself, not helper
// (not null terminated, since it's runtime-const fixed size)
// we bypass dynamic array base because it has two modes:
// - null terminated, where size is computed
// - not null terminated, where size is explicitly tracked
// however, this is a 3rd case where
// - size is 1:1 with max_size() allocated
// we could have pretended we were null-terminated to specialize out size variable, but that's
// misleading and confusing
template <class T, class CharTraits, bool is_const>
class dynamic_array<
        single_fixedbuf_runtimesize_allocator<const T, size_t>,
        experimental::sized_string_policy<CharTraits, int16_t, is_const> >
        : public estd::internal::handle_descriptor_base<
            single_fixedbuf_runtimesize_allocator<const T, size_t>,
            true, true, true, true>
{
    // handle_descriptor_base:
    //  is_singular = true
    //  has_size = true (single fixed buf max_size == size)
    //  is_stateful = true (generally we have a local char* ... though someday we might do a global version)
    typedef estd::internal::handle_descriptor_base<
        single_fixedbuf_runtimesize_allocator<const T, size_t>,
        true, true, true, true> base_t;

public:
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::size_type size_type;
    typedef typename base_t::value_type value_type;

    // never null terminated, so don't bother subtracting off of sizes

    size_type max_size() const { return base_t::get_allocator().max_size(); }
    size_type capacity() const { return base_t::size(); }

    // since we're singular, always pass in 'true' for handle
    template <class TParam>
    dynamic_array(const TParam& p) :
        base_t(p, true) {}

    dynamic_array(const dynamic_array& copy_from) :
        base_t(copy_from.get_allocator(), true)
    {
    }
};
#endif



}}

}
