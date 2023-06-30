/**
 * @file
 * NOTE: My own exposure to embedded development has been 100% 8-bit oriented, so
 * these limits.h reflect that and focus on 8-bit-boundary computing
 */
#pragma once

#include "internal/platform.h"
#include "cstdint.h"
#include "cstddef.h"

// Reduced-dependency flavor - for integral_constant only
// DEBT: Might be better to make a new "traits/type/integral_constant.h" and friends to
// more clearly provide that
#include "internal/type_traits.h"

#ifdef FEATURE_STD_CLIMITS
#include <climits>
#else
// NOTE: Keep an eye on this, make sure we are pulling in proper standard limits.h
// -- and in fact, our filename probably should be climits.h
#include <limits.h>

#endif

#ifndef CHAR_BIT
#warning CHAR_BIT not set, defaulting to 8 bits
#define CHAR_BIT 8
#endif

#if CHAR_BIT != 8
#error "Only 8 bit-wide bytes supported"
#endif

#if FEATURE_CPP_PUSH_MACRO
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min
#endif

namespace estd {

// DEBT: Would likely be better off doing SIZEOF_INTEGER and friends with template
// metaprogramming to avoid potential name collisions
// size in 8-bit bytes
#define SIZEOF_INTEGER(max) \
    (max == INT64_MAX ? 8 : \
    (max == INT32_MAX ? 4 : \
    (max == INT16_MAX ? 2 : 1)))

#define SIZEOF_LLONG    SIZEOF_INTEGER(LLONG_MAX)
#define SIZEOF_LONG     SIZEOF_INTEGER(LONG_MAX)
#define SIZEOF_INT      SIZEOF_INTEGER(INT_MAX)
#define SIZEOF_SHORT    SIZEOF_INTEGER(SHRT_MAX)
#define SIZEOF_CHAR     SIZEOF_INTEGER(CHAR_MAX)

namespace internal {

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

template <class T, bool _is_signed>
struct integer_limits
{
    static CONSTEXPR bool is_specialized = true;

    // non-standard, how many bits in this T
    static CONSTEXPR unsigned bits = (sizeof(T) * CHAR_BIT);

    static CONSTEXPR bool is_integer = true;
    static CONSTEXPR bool is_signed = _is_signed;
    static CONSTEXPR int digits = bits - (is_signed ? 1 : 0);

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

template <class T, bool is_signed_ = true>
struct float_limits
{
    static CONSTEXPR bool is_specialized = true;

    // non-standard, how many bits in this T
    static CONSTEXPR unsigned bits = (sizeof(T) * CHAR_BIT);

    static CONSTEXPR bool is_integer = false;
    static CONSTEXPR bool is_signed = is_signed_;
};

// We maintain this internal one because it helps quite a bit with int -> precision
// mapping.
// DEBT: Just as above, strongly consider making this into 'detail' namespace
template <class T>
struct numeric_limits
{
    static CONSTEXPR bool is_specialized = false;
};

template <>
struct numeric_limits<int8_t> : internal::integer_limits<int8_t, true>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR int8_t min() { return INT8_MIN; }
    static CONSTEXPR int8_t max() { return INT8_MAX; }
};

template <>
struct numeric_limits<uint8_t> : internal::integer_limits<uint8_t, false>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR uint8_t min() { return 0; }
    static CONSTEXPR uint8_t max() { return UINT8_MAX; }
};

template <>
struct numeric_limits<int16_t> : internal::integer_limits<int16_t, true>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR int16_t min() { return INT16_MIN; }
    static CONSTEXPR int16_t max() { return INT16_MAX; }
};

template <>
struct numeric_limits<uint16_t> : internal::integer_limits<uint16_t, false>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR uint16_t min() { return 0; }
    static CONSTEXPR uint16_t max() { return UINT16_MAX; }
};

template <>
struct numeric_limits<int32_t> : internal::integer_limits<int32_t, true>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR int32_t min() { return INT32_MIN; }
    static CONSTEXPR int32_t max() { return INT32_MAX; }
};

template <>
struct numeric_limits<uint32_t> : internal::integer_limits<uint32_t, false>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR uint32_t min() { return 0; }
    static CONSTEXPR uint32_t max() { return UINT32_MAX; }
};

template <>
struct numeric_limits<int64_t> : internal::integer_limits<int64_t, true>
{
    static CONSTEXPR int digits10 = 18;
    static CONSTEXPR int64_t min() { return INT64_MIN; }
    static CONSTEXPR int64_t max() { return INT64_MAX; }
};

template <>
struct numeric_limits<uint64_t> :  internal::integer_limits<uint64_t, false>
{
    static CONSTEXPR int digits10 = 19;
    static CONSTEXPR uint64_t min() { return 0; }
    static CONSTEXPR uint64_t max() { return UINT64_MAX; }
};

// NOTE: Just discovered the __SIZEOF_xxx__ paradigm, see if I can pare down my own version
// Basically experimental
#ifdef __SIZEOF_INT128__
template <>
struct numeric_limits<__int128> :  internal::integer_limits<__int128, true>
{
    static CONSTEXPR int digits10 = 19; // FIX: this is wrong, represents 64-bit size
    static CONSTEXPR uint64_t min() { return 0; }
    // FIX: the following is just an approximation for now
    static CONSTEXPR uint64_t max() { return UINT64_MAX * ((UINT64_MAX) / 2); }
};
#endif

#ifdef __FLT_MIN__
template <>
struct numeric_limits<float> : internal::float_limits<float>
{
    static ESTD_CPP_CONSTEXPR_RET float lowest()
    {
        return -__FLT_MAX__;
    }

    static ESTD_CPP_CONSTEXPR_RET float min()
    {
        return __FLT_MIN__;
    }

    static ESTD_CPP_CONSTEXPR_RET float max()
    {
        return __FLT_MAX__;
    }
};
#endif

#ifdef __DBL_MIN__
template <>
struct numeric_limits<double> : internal::float_limits<double>
{
    static ESTD_CPP_CONSTEXPR_RET double lowest()
    {
        return -__DBL_MAX__;
    }

    static ESTD_CPP_CONSTEXPR_RET double min()
    {
        return __DBL_MIN__;
    }

    static ESTD_CPP_CONSTEXPR_RET double max()
    {
        return __DBL_MAX__;
    }
};
#endif


}

template <class T> struct numeric_limits;


#if SIZEOF_CHAR == 16
template <> struct numeric_limits<signed char> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned char> : internal::numeric_limits<uint16_t> {};
#else
template <> struct numeric_limits<signed char> : internal::numeric_limits<int8_t> {};
template <> struct numeric_limits<unsigned char> : internal::numeric_limits<uint8_t> {};
#endif

#if SIZEOF_SHORT == 4
template <> struct numeric_limits<short> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned short> : internal::numeric_limits<uint32_t> {};
#else
template <> struct numeric_limits<short> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned short> : internal::numeric_limits<uint16_t> {};
#endif


#if SIZEOF_INT == 8
template <> struct numeric_limits<int> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint64_t> {};
#elif SIZEOF_INT == 4
template <> struct numeric_limits<int> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint32_t> {};
#else
template <> struct numeric_limits<int> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint16_t> {};
#endif


#if SIZEOF_LONG == 8
template <> struct numeric_limits<long> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned long> : internal::numeric_limits<uint64_t> {};
#else
template <> struct numeric_limits<long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long> : internal::numeric_limits<uint32_t> {};
#endif

#if SIZEOF_LLONG == 8
template <> struct numeric_limits<long long> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint64_t> {};
#else
template <> struct numeric_limits<long long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint32_t> {};
#endif

template <> struct numeric_limits<bool>
{
    static CONSTEXPR bool min() { return false; }
    static CONSTEXPR bool max() { return true; }

    static CONSTEXPR int digits = 1;
    static CONSTEXPR int digits10 = 0;
};

}

#if FEATURE_CPP_PUSH_MACRO
#pragma pop_macro("min")
#pragma pop_macro("max")
#endif
