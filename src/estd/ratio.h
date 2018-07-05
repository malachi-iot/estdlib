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

}
