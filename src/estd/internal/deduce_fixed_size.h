//
// Created by malachi on 7/17/18.
//
#pragma once

#include <stdint.h> // for uint8_t and friends
#include "../cstddef.h" // for size_t/ptrdiff_t

namespace estd {
namespace internal {

// lifted from https://stackoverflow.com/questions/9510514/integer-range-based-template-specialisation
template<bool>
struct Range;

template<size_t val, typename = Range<true> >
class deduce_fixed_size_t
{
};

template<size_t val>
struct deduce_fixed_size_t<val, Range<(0 <= val && val <= 255)> >
{
    typedef uint8_t size_type;
};

template<size_t val>
struct deduce_fixed_size_t<val, Range<(255 < val && val <= 0xFFFF)> >
{
    typedef uint16_t size_type;
};

template<size_t val>
struct deduce_fixed_size_t<val, Range<(0xFFFF < val && val <= 0xFFFFFFFF)> >
{
    typedef uint32_t size_type;
};


template <ptrdiff_t val, typename = Range<true> >
class deduce_bit_count {};

template<ptrdiff_t val>
struct deduce_bit_count<val, Range<(0 <= val && val <= 1)> >
{
    static CONSTEXPR uint8_t value = 1;
};

template<ptrdiff_t val>
struct deduce_bit_count<val, Range<(2 <= val && val <= 3)> >
{
    static CONSTEXPR uint8_t value = 2;
};

template<ptrdiff_t val>
struct deduce_bit_count<val, Range<(4 <= val && val <= 7)> >
{
    static CONSTEXPR uint8_t value = 3;
};


template<ptrdiff_t val>
struct deduce_bit_count<val, Range<(8 <= val && val <= 15)> >
{
    static CONSTEXPR uint8_t value = 4;
};


template<ptrdiff_t val>
struct deduce_bit_count<val, Range<(16 <= val && val <= 31)> >
{
    static CONSTEXPR uint8_t value = 5;
};


template<ptrdiff_t val>
struct deduce_bit_count<val, Range<(32 <= val && val <= 63)> >
{
    static CONSTEXPR uint8_t value = 6;
};


template<ptrdiff_t val>
struct deduce_bit_count<val, Range<(64 <= val && val <= 127)> >
{
    static CONSTEXPR uint8_t value = 7;
};



}
}
