/**
 * @file
 * NOTE: My own exposure to embedded development has been 100% 8-bit oriented, so
 * these limits.h reflect that and focus on 8-bit-boundary computing
 */
#pragma once

#include "internal/feature/estd.h"
#include "internal/limits.h"
#include "cstdint.h"
#include "cstddef.h"

#include "internal/macro/push.h"

namespace estd {

// DEBT: Would likely be better off doing SIZEOF_INTEGER and friends with template
// metaprogramming to avoid potential name collisions
// size in 8-bit bytes
#define SIZEOF_INTEGER(max) \
    (max == INT64_MAX ? 8 : \
    (max == INT32_MAX ? 4 : \
    (max == INT16_MAX ? 2 : 1)))

// TODO: Error if INT64_MAX isn't available

#if !defined(LLONG_WIDTH)

#if __SIZEOF_LONG_LONG__
#define LLONG_WIDTH (__SIZEOF_LONG_LONG__ * 8)

#elif defined(LLONG_MAX)
#define LLONG_WIDTH (SIZEOF_INTEGER(LLONG_MAX) * 8)

#elif ESTD_ARCH_BITNESS == 32
#if FEATURE_ESTD_COMPILE_STRICTNESS > 2
#error Not making assumption about 'long long' for unknown architecture
#elif FEATURE_ESTD_COMPILE_VERBOSITY > 1
#warning Assuming long long is 64 bit
#endif
#define LLONG_WIDTH     64

#elif ESTD_ARCH_BITNESS != 64 && FEATURE_ESTD_COMPILE_VERBOSITY > 0
#warning Not making assumption about 'long long' for unknown architecture
#endif
#endif

#ifndef __SIZEOF_LONG_LONG__
#define __SIZEOF_LONG_LONG__    (LLONG_WIDTH / 8)
#endif
#ifndef __SIZEOF_LONG__
#define __SIZEOF_LONG__         SIZEOF_INTEGER(LONG_MAX)
#endif
#ifndef __SIZEOF_INT__
#define __SIZEOF_INT__          SIZEOF_INTEGER(INT_MAX)
#endif
#ifndef __SIZEOF_SHORT__
#define __SIZEOF_SHORT__        SIZEOF_INTEGER(SHRT_MAX)
#endif


// As per https://en.cppreference.com/w/c/types/limits and
// https://pubs.opengroup.org/onlinepubs/009695399/basedefs/limits.h.html
#ifndef CHAR_WIDTH
#define CHAR_WIDTH      CHAR_BIT
#endif
#ifndef SHRT_WIDTH
#define SHRT_WIDTH      (__SIZEOF_SHORT__ * 8)
#endif
#ifndef INT_WIDTH
#define INT_WIDTH       (__SIZEOF_INT__ * 8)
#endif
#ifndef LONG_WIDTH
#define LONG_WIDTH      (__SIZEOF_LONG__ * 8)
#endif

// Long long already handled up above

namespace internal {

// We maintain this internal one because it helps quite a bit with int -> precision
// mapping.
// DEBT: Just as above, strongly consider making this into 'detail' namespace
template <class T> struct numeric_limits;

template <>
struct numeric_limits<int8_t> : integer_limits<int8_t, true>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR int8_t min() { return INT8_MIN; }
    static CONSTEXPR int8_t max() { return INT8_MAX; }

    typedef uint8_t unsigned_type;
};

template <>
struct numeric_limits<uint8_t> : integer_limits<uint8_t, false>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR uint8_t min() { return 0; }
    static CONSTEXPR uint8_t max() { return UINT8_MAX; }

    typedef int8_t signed_type;
};

template <>
struct numeric_limits<int16_t> : integer_limits<int16_t, true>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR int16_t min() { return INT16_MIN; }
    static CONSTEXPR int16_t max() { return INT16_MAX; }

    typedef uint16_t unsigned_type;
};

template <>
struct numeric_limits<uint16_t> : integer_limits<uint16_t, false>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR uint16_t min() { return 0; }
    static CONSTEXPR uint16_t max() { return UINT16_MAX; }

    typedef int16_t signed_type;
};

template <>
struct numeric_limits<int32_t> : integer_limits<int32_t, true>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR int32_t min() { return INT32_MIN; }
    static CONSTEXPR int32_t max() { return INT32_MAX; }

    typedef uint32_t unsigned_type;
};

template <>
struct numeric_limits<uint32_t> : integer_limits<uint32_t, false>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR uint32_t min() { return 0; }
    static CONSTEXPR uint32_t max() { return UINT32_MAX; }

    typedef int32_t signed_type;
};

template <>
struct numeric_limits<int64_t> : integer_limits<int64_t, true>
{
    static CONSTEXPR int digits10 = 18;
    static CONSTEXPR int64_t min() { return INT64_MIN; }
    static CONSTEXPR int64_t max() { return INT64_MAX; }

    typedef uint64_t unsigned_type;
};

template <>
struct numeric_limits<uint64_t> :  integer_limits<uint64_t, false>
{
    static CONSTEXPR int digits10 = 19;
    static CONSTEXPR uint64_t min() { return 0; }
    static CONSTEXPR uint64_t max() { return UINT64_MAX; }

    typedef int64_t signed_type;
};

#ifdef __SIZEOF_INT128__
template <>
struct numeric_limits<__int128> :  integer_limits<__int128, true>
{
    static CONSTEXPR int digits10 = 19; // FIX: this is wrong, represents 64-bit size
    static CONSTEXPR uint64_t min() { return 0; }
    // FIX: the following is just an approximation for now
    static CONSTEXPR __int128 max() { return UINT64_MAX * ((UINT64_MAX) / 2); }
};
#endif

}

// SIZEOF_CHAR "is always 1" https://isocpp.org/wiki/faq/intrinsic-types#sizeof-char
template <> struct numeric_limits<signed char> : internal::numeric_limits<int8_t> {};
template <> struct numeric_limits<unsigned char> : internal::numeric_limits<uint8_t> {};

#if SHORT_WIDTH == 32
template <> struct numeric_limits<short> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned short> : internal::numeric_limits<uint32_t> {};
#else
template <> struct numeric_limits<short> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned short> : internal::numeric_limits<uint16_t> {};
#endif


#if INT_WIDTH == 64
template <> struct numeric_limits<int> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint64_t> {};
#elif INT_WIDTH == 32
template <> struct numeric_limits<int> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint32_t> {};
#elif INT_WIDTH == 16
template <> struct numeric_limits<int> : internal::numeric_limits<int16_t> {};
template <> struct numeric_limits<unsigned int> : internal::numeric_limits<uint16_t> {};
#else
#warning Unexpected integer size, not specialized
#endif


#if LONG_WIDTH == 64
template <> struct numeric_limits<long> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned long> : internal::numeric_limits<uint64_t> {};
#elif LONG_WIDTH == 32
template <> struct numeric_limits<long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long> : internal::numeric_limits<uint32_t> {};
#else
#error Failed SIZEOF_LONG sanity check
#endif

#if LLONG_WIDTH == 64
template <> struct numeric_limits<long long> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint64_t> {};
#elif LLONG_WIDTH == 32
// DEBT: I am not convinced any has a 32-bit long long, and I am not convinced anyone should
template <> struct numeric_limits<long long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint32_t> {};
#elif FEATURE_ESTD_COMPILE_VERBOSITY > 2
#warning Failed SIZEOF_LLONG sanity check
#endif

template <> struct numeric_limits<bool>
{
    static CONSTEXPR bool min() { return false; }
    static CONSTEXPR bool max() { return true; }

    static CONSTEXPR int digits = 1;
    static CONSTEXPR int digits10 = 0;
};

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

    static CONSTEXPR unsigned digits = FLT_MANT_DIG;
    static CONSTEXPR unsigned digits10 = FLT_DIG;
#if FLT_DECIMAL_DIG
    static CONSTEXPR unsigned max_digita10 = FLT_DECIMAL_DIG;
#endif
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

    static CONSTEXPR unsigned digits = DBL_MANT_DIG;
    static CONSTEXPR unsigned digits10 = DBL_DIG;
#if DBL_DECIMAL_DIG
    static CONSTEXPR unsigned max_digita10 = DBL_DECIMAL_DIG;
#endif
};
#endif

}

#include "internal/macro/pop.h"

