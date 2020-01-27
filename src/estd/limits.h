/**
 * @file
 * NOTE: My own exposure to embedded development has been 100% 8-bit oriented, so
 * these limits.h reflect that and focus on 8-bit-boundary computing
 */
#pragma once

#include "internal/platform.h"
#include "cstdint.h"
#include "cstddef.h"

#ifdef FEATURE_STD_CLIMITS
#include <climits>
#else
// NOTE: Keep an eye on this, make sure we are pulling in proper standard limits.h
// -- and in fact, our filename probably should be climits.h
#include <limits.h>

#endif

#if CHAR_BIT != 8
#error "Only 8 bit-wide bytes supported"
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

template <class T, bool _is_signed>
struct integer_limits
{
    static CONSTEXPR bool is_integer = true;
    static CONSTEXPR bool is_signed = _is_signed;
    static CONSTEXPR int digits = (sizeof(T) * 8) - (is_signed ? 1 : 0);
};


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


}

#if defined(__GNUC__) || defined(_MSC_VER)
#pragma pop_macro("min")
#pragma pop_macro("max")
#endif
