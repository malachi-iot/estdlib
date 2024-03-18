#pragma once

#include "../fwd/limits.h"
#include "../type_traits.h"

#include "macros.h"

namespace estd { namespace internal {

// DEBT: Strongly consider uint_strlen and integer_limits as candidates for 'detail' namespace
// they are useful on their own and are approaching stability.  Also, consider a consumer of that
// 'layer1::numeric_string'

template <unsigned base, unsigned bits>
struct uint_strlen;

// DEBT: I'm sure there's a more math-y way to do this

template <> struct uint_strlen<10, 8> :
    estd::integral_constant<unsigned, 3> {};

template <> struct uint_strlen<10, 16> :
    estd::integral_constant<unsigned, 5> {};

template <> struct uint_strlen<10, 32> :
    estd::integral_constant<unsigned, 10> {};

template <> struct uint_strlen<10, 64> :
    estd::integral_constant<unsigned, 20> {};

template <unsigned bits>
struct uint_strlen<16, bits> :
    estd::integral_constant<unsigned, bits / 4> {};

// DEBT: calculation approximate, higher the bitcount the more wasted space
template <unsigned bits>
struct uint_strlen<8, bits> :
    estd::integral_constant<unsigned, (bits + 1) / 3> {};

// Not the same as boost's version
template <class T, bool is_signed_>
struct signed_traits;

template <class T>
struct signed_traits<T, true>
{
    typedef T signed_type;
};

template <class T>
struct signed_traits<T, false>
{
    typedef T unsigned_type;
};


}}

