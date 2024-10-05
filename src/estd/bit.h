#pragma once

#include "cstdint.h"

namespace estd {

enum class endian
{
// DEBT: Not all _WIN32 are gonna be little endian
#ifdef _WIN32
    little = 0,
    big    = 1,
    native = little
#else
    little = __ORDER_LITTLE_ENDIAN__,
    big    = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__
#endif
};

}

#if __cpp_lib_byteswap >= 202110L
#include <bit>

namespace estd {

using std::byteswap;

}

#else
namespace estd {

template <class Unsigned>
constexpr Unsigned byteswap(Unsigned n) noexcept;

#if __has_builtin(__builtin_bswap16)
template <>
constexpr uint16_t byteswap(uint16_t n) noexcept
{
    return __builtin_bswap16(n);
}
#endif

#if __has_builtin(__builtin_bswap32)
template <>
constexpr uint32_t byteswap(uint32_t n) noexcept
{
    return __builtin_bswap32(n);
}
#endif

#if __has_builtin(__builtin_bswap64)

template <>
constexpr uint64_t byteswap(uint64_t n) noexcept
{
#if __has_builtin(__builtin_bswap32)
    return __builtin_bswap64(n);
#else
    return 0;
#endif
}
#endif

}

#endif