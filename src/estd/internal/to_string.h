#pragma once

// string.h includes US, so this should never actually include anything,
// just using it for tooltip help
#include "estd/string.h"

// not doing #include <stdio.h> because all its putc/putchar macros get things
// confused
extern "C" int sprintf ( char * str, const char * format, ... );

namespace estd {

// TODO: wait until the util.embedded length-helpers (and toString helpers) are around to really build this out a lot
inline layer1::string<12> to_string(int value)
{
    layer1::string<12> val;

    sprintf(val.lock(), "%d", value);
    val.unlock();

    return val;
}

}
