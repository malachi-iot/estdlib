//
// Created by malachi on 7/17/18.
//
#pragma once

#include <stdint.h> // for uint8_t and friends

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


}
}