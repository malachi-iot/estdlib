#pragma once

#include "internal/platform.h"
#include "cstdint.h"

#ifdef FEATURE_STD_CLIMITS
#include <climits>
#else
// NOTE: My own exposure to embedded development has been 100% 8-bit oriented, so
// these limits.h reflect that and focus on 8-bit-boundary computing

// TODO: this actually should be from a climits/climits.h
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif
#endif

#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min
#endif

namespace estd {

// FIX: I implemented this wrong, numeric_limits should be a specializing type, not a namespace
// holding on to incorrect one in deprecated form just incase I used it somewhere
namespace numeric_limits_deprecated {

template <class T>
CONSTEXPR T max();

template<>
inline CONSTEXPR uint16_t max<uint16_t>()
{ return 65535; }

template<>
inline CONSTEXPR uint8_t max<uint8_t>()
{ return 255; }

}

namespace internal {

template <class T, bool _is_signed>
struct integer_limits
{
    static CONSTEXPR bool is_integer = true;
    static CONSTEXPR bool is_signed = _is_signed;
    static CONSTEXPR int digits = (sizeof(T) * 8) - (is_signed ? 1 : 0);
};

}

template <class T> struct numeric_limits;

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

// TODO: Look into why all the above uint flavors don't cover these embedded
// targets
#ifdef __ADSPBLACKFIN__
// ?? thought this would be covered by uint32_t, guess not
template <>
struct numeric_limits<unsigned long> : numeric_limits<uint32_t> {};
#elif defined(__arm__)
// http://www.keil.com/support/man/docs/armcc/armcc_chr1359125009502.htm
// https://stackoverflow.com/questions/23934862/what-predefined-macro-can-i-use-to-detect-the-target-architecture-in-clang/41666292
template <>
struct numeric_limits<int> : numeric_limits<int32_t> {};
template <>
struct numeric_limits<unsigned int> : numeric_limits<uint32_t> {};
#endif

// FIX: CLang not playing nice here, yet
#ifdef ULONG_MAX
template <>
struct numeric_limits<unsigned long> : internal::integer_limits<unsigned long, false>
{
    static CONSTEXPR unsigned long min() { return 0; }
    static CONSTEXPR unsigned long max() { return ULONG_MAX; }
};
#endif

#ifdef INT64_MAX

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

#endif

}

#if defined(__GNUC__) || defined(_MSC_VER)
#pragma pop_macro("min")
#pragma pop_macro("max")
#endif