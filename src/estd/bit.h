#pragma once

#include "cstdint.h"
#include "internal/feature/bit.h"

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

#if (FEATURE_ESTD_BYTESWAP_MODE == FEATURE_ESTD_BYTESWAP_AUTO && __cpp_lib_byteswap >= 202110L) || \
    FEATURE_ESTD_BYTESWAP_MODE == FEATURE_ESTD_BYTESWAP_STD
#include <bit>

namespace estd {

using std::byteswap;

}

#else
namespace estd {

namespace detail {
#if FEATURE_ESTD_BYTESWAP_MODE == FEATURE_ESTD_BYTESWAP_XOR

#error manual xor non gcc/ non std mode not yet supported

#else // not xor (gcc)

#if FEATURE_TOOLCHAIN_BUILTIN_BSWAP16 || FEATURE_ESTD_BYTESWAP_MODE == FEATURE_ESTD_BYTESWAP_GCC

constexpr uint16_t byteswap(uint16_t n) noexcept
{
    return __builtin_bswap16(n);
}

// ISO/IEC 9899:201x aka WG14 N1570 (https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
// Section 6.2.6.2 indicates one signed bit is present, and section 3.4.1 immplies it's always
// the high order bit.  This gives us confidence that we can swap bits around safely, trusting that
// whoever "unswaps" them will end up with the expected high order signed bit.

constexpr int16_t byteswap(int16_t n) noexcept
{
    return __builtin_bswap16(n);
}

#endif

#if FEATURE_TOOLCHAIN_BUILTIN_BSWAP32 || FEATURE_ESTD_BYTESWAP_MODE == FEATURE_ESTD_BYTESWAP_GCC

constexpr uint32_t
byteswap(uint32_t n) noexcept
{
    return __builtin_bswap32(n);
}

constexpr int32_t
byteswap(int32_t n) noexcept
{
    return __builtin_bswap32(n);
}

#endif

#if FEATURE_TOOLCHAIN_BUILTIN_BSWAP64 || FEATURE_ESTD_BYTESWAP_MODE == FEATURE_ESTD_BYTESWAP_GCC

constexpr uint64_t
byteswap(uint64_t n) noexcept
{
    return __builtin_bswap64(n);
}

#endif

#endif // not xor (gcc)
}

// Since spec indicates a templated function, we wrap up the specific ones
// in detail.  This is better than specializing, since that can lead to linker ambiguities
template <class T>
constexpr T byteswap(T n) noexcept { return detail::byteswap(n); }

}



#endif