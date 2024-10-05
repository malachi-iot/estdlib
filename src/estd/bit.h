#pragma once

namespace estd {

enum class endian
{
// DEBT: Not all _WIN32 are gonna be little endian
#ifdef _WIN32
    little = 0,
    big    = 1,
    native = little
#else
    little = __ORDER_LITTLE_ENDIAN__,
    big    = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__
#endif
};


}