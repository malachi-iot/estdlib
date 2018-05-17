#pragma once

// string.h includes US, so this should never actually include anything,
// just using it for tooltip help
#include "estd/string.h"

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