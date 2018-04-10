#pragma once

#include <stdlib.h>

// TODO: utilize portions of std array here, if we can
// note that std::array maps directly to our layer1 approach
// but we value add with layer2, layer3, etc.

namespace estd {

template<
    class T,
    std::size_t N
> struct array;

}
