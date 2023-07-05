#pragma once

#include "../platform.h"

namespace estd {

template <class T>
struct numeric_limits
{
    static CONSTEXPR bool is_specialized = false;
    static CONSTEXPR bool is_integer = false;
    static CONSTEXPR bool is_iec559 = false;
};

}
