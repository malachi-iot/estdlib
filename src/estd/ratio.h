#pragma once

#include "internal/platform.h"
#include <cstdint>

namespace estd {

template<
    std::intmax_t Num,
    std::intmax_t Denom = 1
> class ratio
{
public:
    static CONSTEXPR std::intmax_t num() { return Num; }
    static CONSTEXPR std::intmax_t den() { return Denom; }
};

typedef ratio<1,1000000000> nano;
typedef ratio<1,1000000> micro;
typedef ratio<1,1000> milli;
typedef ratio<1, 10> deci;
typedef ratio<1000, 1> kilo;
typedef ratio<1000000, 1> mega;
typedef ratio<1000000000, 1> giga;


}
