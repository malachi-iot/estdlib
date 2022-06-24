#pragma once

#include "platform.h"

namespace estd {

namespace internal {

// https://stackoverflow.com/questions/43846187/using-template-metaprogramming-in-c-find-the-gcd-of-two-integers
template<std::intmax_t a, std::intmax_t b> struct gcd
{
    static CONSTEXPR std::intmax_t value = gcd<b, a % b>::value;
};

template<std::intmax_t a>
struct gcd<a, 0>
{
    static CONSTEXPR std::intmax_t value = a;
};

// https://www.variadic.xyz/2012/01/07/c-template-metaprogramming/
template<std::intmax_t One, std::intmax_t Two>
struct lcm
{
    enum
    {
        value = One * Two / gcd<One, Two>::value
    };
};


}


}