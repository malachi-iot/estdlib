#pragma once

#include "internal/platform.h"

namespace estd {

#ifdef FEATURE_CPP_ENUM_CLASS
enum class byte : unsigned char {};
#else
typedef unsigned char byte;
#endif

template <class IntegerType>
CONSTEXPR IntegerType to_integer(byte b) noexcept
{
    // TODO: Only allow overloads which conform to is_integral, as per spec
    //is_integral<IntegerType>::value;
    return IntegerType(b);
}


/* TODO: Figure out how to do this; not critical for my own uses yet
template <class IntegerType>
CONSTEXPR byte& operator >>=(byte& b, IntegerType shift) noexcept
{
    return (b = ((unsigned char)b) >> shift);
} */

}

#ifdef FEATURE_STD_CSTDDEF
#include <cstddef>
#else
#include <stddef.h>
namespace std {

typedef ::size_t size_t;
typedef ::ptrdiff_t ptrdiff_t;

}
#endif
