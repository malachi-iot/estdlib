/**
 * @file
 * NOTE: My own exposure to embedded development has been 100% 8-bit oriented, so
 * these limits.h reflect that and focus on 8-bit-boundary computing
 */
#pragma once

#include "internal/limits.h"
#include "cstdint.h"
#include "cstddef.h"

// Reduced-dependency flavor - for integral_constant only
// DEBT: Might be better to make a new "traits/type/integral_constant.h" and friends to
// more clearly provide that
#include "internal/type_traits.h"

#if FEATURE_CPP_PUSH_MACRO
#include "internal/macro/push.h"
#endif

namespace estd {

// DEBT: Would likely be better off doing SIZEOF_INTEGER and friends with template
// metaprogramming to avoid potential name collisions
// size in 8-bit bytes
#define SIZEOF_INTEGER(max) \
    (max == INT64_MAX ? 8 : \
    (max == INT32_MAX ? 4 : \
    (max == INT16_MAX ? 2 : 1)))

// TODO: Use ESTD_ARCH_BITNESS to assist here -- 32 bit targets let's
// presume a 64-bit long long exists, but do give a warning
#if __AVR__ || ESP_PLATFORM
// DEBT: Pretty clunky.  __AVR__ & ESP32 target seems to have 64-bit support
// mapped to "long long", but it is missing this crucial macro
#define LLONG_WIDTH     64
#endif

// DEBT: Move away from SIZEOF_xxx in favor of standards-based xxx_WIDTH
#ifdef LLONG_MAX
#define SIZEOF_LLONG    SIZEOF_INTEGER(LLONG_MAX)
#elif defined(LLONG_WIDTH)
#define SIZEOF_LLONG    (LLONG_WIDTH / 8)
#endif
#define SIZEOF_LONG     SIZEOF_INTEGER(LONG_MAX)
#define SIZEOF_INT      SIZEOF_INTEGER(INT_MAX)
#define SIZEOF_SHORT    SIZEOF_INTEGER(SHRT_MAX)
#define SIZEOF_CHAR     SIZEOF_INTEGER(CHAR_MAX)

// TODO: Check against _ISOC2X_SOURCE
#ifndef SHRT_WIDTH
#define SHRT_WIDTH      (8*SIZEOF_SHORT)
#define INT_WIDTH       (8*SIZEOF_INT)
#define LONG_WIDTH      (8*SIZEOF_LONG)
#endif
#if !defined(LLONG_WIDTH) && defined(SIZEOF_LLONG)
#define LLONG_WIDTH     (8*SIZEOF_LLONG)
#endif

namespace internal {

// We maintain this internal one because it helps quite a bit with int -> precision
// mapping.
// DEBT: Just as above, strongly consider making this into 'detail' namespace
template <class T> struct numeric_limits;

template <>
struct numeric_limits<int8_t> : internal::integer_limits<int8_t, true>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR int8_t min() { return INT8_MIN; }
    static CONSTEXPR int8_t max() { return INT8_MAX; }

    typedef uint8_t unsigned_type;
};

template <>
struct numeric_limits<uint8_t> : internal::integer_limits<uint8_t, false>
{
    static CONSTEXPR int digits10 = 2;
    static CONSTEXPR uint8_t min() { return 0; }
    static CONSTEXPR uint8_t max() { return UINT8_MAX; }

    typedef int8_t signed_type;
};

template <>
struct numeric_limits<int16_t> : internal::integer_limits<int16_t, true>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR int16_t min() { return INT16_MIN; }
    static CONSTEXPR int16_t max() { return INT16_MAX; }

    typedef uint16_t unsigned_type;
};

template <>
struct numeric_limits<uint16_t> : internal::integer_limits<uint16_t, false>
{
    static CONSTEXPR int digits10 = 4;
    static CONSTEXPR uint16_t min() { return 0; }
    static CONSTEXPR uint16_t max() { return UINT16_MAX; }

    typedef int16_t signed_type;
};

template <>
struct numeric_limits<int32_t> : internal::integer_limits<int32_t, true>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR int32_t min() { return INT32_MIN; }
    static CONSTEXPR int32_t max() { return INT32_MAX; }

    typedef uint32_t unsigned_type;
};

template <>
struct numeric_limits<uint32_t> : internal::integer_limits<uint32_t, false>
{
    static CONSTEXPR int digits10 = 9;
    static CONSTEXPR uint32_t min() { return 0; }
    static CONSTEXPR uint32_t max() { return UINT32_MAX; }

    typedef int32_t signed_type;
};

template <>
struct numeric_limits<int64_t> : internal::integer_limits<int64_t, true>
{
    static CONSTEXPR int digits10 = 18;
    static CONSTEXPR int64_t min() { return INT64_MIN; }
    static CONSTEXPR int64_t max() { return INT64_MAX; }

    typedef uint64_t unsigned_type;
};

template <>
struct numeric_limits<uint64_t> :  internal::integer_limits<uint64_t, false>
{
    static CONSTEXPR int digits10 = 19;
    static CONSTEXPR uint64_t min() { return 0; }
    static CONSTEXPR uint64_t max() { return UINT64_MAX; }

    typedef int64_t signed_type;
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

}

// DEBT: Innocuous code, but incorrect.
// SIZEOF_CHAR "is always 1" https://isocpp.org/wiki/faq/intrinsic-types#sizeof-char
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
#elif SIZEOF_LONG == 4
template <> struct numeric_limits<long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long> : internal::numeric_limits<uint32_t> {};
#else
#error Failed SIZEOF_LONG sanity check
#endif

#if SIZEOF_LLONG == 8
template <> struct numeric_limits<long long> : internal::numeric_limits<int64_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint64_t> {};
#elif SIZEOF_LLONG == 4
// DEBT: I am not convinced any has a 32-bit long long, and I am not convinced anyone should
template <> struct numeric_limits<long long> : internal::numeric_limits<int32_t> {};
template <> struct numeric_limits<unsigned long long> : internal::numeric_limits<uint32_t> {};
#else
#error Failed SIZEOF_LLONG sanity check
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

#if FEATURE_CPP_PUSH_MACRO
#include "internal/macro/pop.h"
#endif
