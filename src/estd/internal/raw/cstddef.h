/**
 * @file
 * standalone implementation so that can be #included from support_platform.h
 */
#pragma once

namespace estd {

#ifdef FEATURE_CPP_ENUM_CLASS
enum class byte : unsigned char {};
#else
// non-member operator overloads want a full struct or enum
struct byte
{
    unsigned char value;

    byte() {}

    byte(unsigned char value) : value(value) {}

    operator unsigned char() const { return value; }
};
#endif

// TODO: Only allow overloads which conform to is_integral, as per spec

template <class IntegerType>
//#if defined(FEATURE_CPP_DEDUCE_RETURN) and defined(FEATURE_CPP_CONSTEXPR)
//constexpr auto to_integer(byte b) noexcept -> is_integral<IntegerType>::
//#else
CONSTEXPR IntegerType to_integer(byte b) NOEXCEPT
//#endif
{
    //is_integral<IntegerType>::value;
    return IntegerType(b);
}


// NONSTANDARD
// Because most of us aren't running C++17 yet with relaxed rules to do the
// canonical byte v={0} or similar, so do this in the meantime
template <class IntegerType>
CONSTEXPR byte to_byte(IntegerType value) NOEXCEPT
{
#ifdef FEATURE_CPP_ENUM_CLASS
    return (byte) value;
#else
    return byte(value);
#endif
}


template <class IntegerType>
CONSTEXPR byte operator <<(byte b, IntegerType shift) NOEXCEPT
{
    return (byte)((unsigned char)b << shift);
    //return b;
}

template <class IntegerType>
CONSTEXPR byte& operator >>=(byte& b, IntegerType shift) NOEXCEPT
{
    //return ((unsigned char&)b) >>= shift;
    return b = (byte)(((unsigned char)b) >> shift);
    //return b;
}


template <class IntegerType>
CONSTEXPR byte& operator <<=(byte& b, IntegerType shift) NOEXCEPT
{
    return b = b << shift;
    //return b = (byte)(((unsigned char)b) << shift);
    //return b;
}


inline CONSTEXPR byte operator|(byte l, byte r) NOEXCEPT
{
    return to_byte(static_cast<unsigned int>(l) | static_cast<unsigned int>(r));
}

inline
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#else
const
#endif
byte& operator|=(byte& l, byte r) NOEXCEPT
{
    return l = l | r;
}


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
