#pragma once

#include "../platform.h"

namespace estd {

// TODO: Consider a strlen_c or similar using recursion.  That might accommodate a c++11
// constexpr requirement
template <class Char>
ESTD_CPP_CONSTEXPR(14) size_t strlen(const Char* s)
{
    const Char* i = s;

    while(*i != 0)  ++i;

    return i - s;
}

}
