#pragma once

#include "platform.h"
#include "fwd/limits.h"
#include "type_traits.h"

#ifdef FEATURE_STD_CLIMITS
#include <climits>
#include <cfloat>
#else
// NOTE: Keep an eye on this, make sure we are pulling in proper standard limits.h
// -- and in fact, our filename probably should be climits.h
#include <limits.h>
#include <float.h>

#endif

#ifndef CHAR_BIT
#if FEATURE_ESTD_COMPILE_VERBOSITY > 1
#warning CHAR_BIT not set, defaulting to 8 bits
#endif
#define CHAR_BIT 8
#endif

#if CHAR_BIT != 8
#error "Only 8 bit-wide bytes supported"
#endif

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


template <class T, bool is_signed_>
struct integer_limits : signed_traits<T, is_signed_>
{
    static CONSTEXPR bool is_specialized = true;

    // non-standard, how many bits in this T
    static CONSTEXPR unsigned bits = (sizeof(T) * CHAR_BIT);

    static CONSTEXPR bool is_integer = true;
    static CONSTEXPR bool is_signed = is_signed_;
    static CONSTEXPR int digits = bits - (is_signed ? 1 : 0);
    static CONSTEXPR bool is_iec559 = false;
    static CONSTEXPR unsigned radix = 2;

    /// Retrieves maximum length a string of this int, accounting for
    /// base representation
    /// NOTE: this is an estd extension, not part of std
    /// DEBT: Only base 10 minus sign SHOULD be considered, but char conversion code doesn't
    /// filter out the notion of negative hex, for example.  That really does need to be sorted out,
    /// but in the meantime we overallocate here to avoid a stack corruption
    /// \tparam base supported values: 8, 10, 16
    template <unsigned base>
    struct length :
        integral_constant<unsigned, uint_strlen<base, bits>::value +
                                    //((is_signed && base == 10) ? 1 : 0)> {};
                                    (is_signed ? 1 : 0)> {};
};

// NOTE: Did anyone ever hear of a non-signed floating point type?
template <class T, bool is_signed_ = true>
struct float_limits
{
    static CONSTEXPR bool is_specialized = true;

    // non-standard, how many bits in this T
    static CONSTEXPR unsigned bits = (sizeof(T) * CHAR_BIT);

    static CONSTEXPR bool is_integer = false;
    static CONSTEXPR bool is_signed = is_signed_;
    static CONSTEXPR unsigned radix = FLT_RADIX;

#if defined(__GCC_IEC_559) || defined(__STDC_IEC_559__)
    static CONSTEXPR bool is_iec559 = true;

#if FEATURE_ESTD_DRAGONBOX
    // NOTE: Shamelessly reworked from Dragonbox' max_output_string_length
    // DEBT: For 32 bit, is one byte too large due to exp len of 3 when it
    // should be 2
    template <class Format = jkj::dragonbox::default_float_traits<T> >
    struct length_exp :
        integral_constant<unsigned,
            // sign(1) + significand(9) + decimal_point + exp_marker + exp_sign + exp
            1 + Format::decimmal_digits + 1 + 1 + 3>::value >;
#endif

#else
    static CONSTEXPR bool is_iec559 = false;
#endif

};

}}
