#pragma once

#include "fwd.h"

namespace estd { namespace experimental {

struct ctype_base
{
    typedef uint8_t mask;

    static CONSTEXPR mask space = 0x01;
    static CONSTEXPR mask digit = 0x02;
    static CONSTEXPR mask alpha = 0x04;
    static CONSTEXPR mask punct = 0x08;
    static CONSTEXPR mask upper = 0x10;
    static CONSTEXPR mask lower = 0x20;
    static CONSTEXPR mask xdigit = 0x40;

    static CONSTEXPR mask alnum = alpha | digit;
    static CONSTEXPR mask graph = alnum | punct;
};


}}