#pragma once

#include "../../cstdint.h"

namespace estd {

template<
    std::intmax_t Num,
    std::intmax_t Denom = 1
> class ratio;

namespace detail {

template <class R1, class R2>
struct ratio_add;

template <class R1, class R2>
struct ratio_divide;

template <class R1, class R2>
struct ratio_multiply;

}

}