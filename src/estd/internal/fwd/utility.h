#pragma once

#include "../macro/c++/const.h"

namespace estd {

template<
    class T1,
    class T2
> struct pair;

template <class T>
ESTD_CPP_CONSTEXPR(14) void swap(T& a, T& b);

}
