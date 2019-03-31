#pragma once

#include "internal/platform.h"
#include "cstdint.h"

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

template <class T> struct numeric_limits;

template <>
struct numeric_limits<int8_t>
{
    static int8_t min() { return INT8_MIN; }
    static int8_t max() { return INT8_MAX; }
};

template <>
struct numeric_limits<uint8_t>
{
    static uint8_t min() { return 0; }
    static uint8_t max() { return UINT8_MAX; }
};

template <>
struct numeric_limits<int16_t>
{
    static int16_t min() { return INT16_MIN; }
    static int16_t max() { return INT16_MAX; }
};

template <>
struct numeric_limits<uint16_t>
{
    static uint16_t min() { return 0; }
    static uint16_t max() { return UINT16_MAX; }
};

template <>
struct numeric_limits<int32_t>
{
    static int32_t min() { return INT32_MIN; }
    static int32_t max() { return INT32_MAX; }
};

template <>
struct numeric_limits<uint32_t>
{
    static uint32_t min() { return 0; }
    static uint32_t max() { return UINT32_MAX; }
};

#ifdef INT64_MAX

template <>
struct numeric_limits<int64_t>
{
    static int64_t min() { return INT64_MIN; }
    static int64_t max() { return INT64_MAX; }
};

template <>
struct numeric_limits<uint64_t>
{
    static uint64_t min() { return 0; }
    static uint64_t max() { return UINT64_MAX; }
};

#endif

}
