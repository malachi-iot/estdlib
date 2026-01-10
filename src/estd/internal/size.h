#pragma once

// In support of:
// https://github.com/malachi-iot/estdlib/issues/166
// https://en.cppreference.com/w/cpp/iterator/size.html

#include "iterator_standalone.h"

namespace estd {

namespace internal {

// Underlying feeder for begin, end, size
template <class Container>
struct container_traits
{
    using container_type = Container;
    using iterator = typename container_type::pointer;
};

}

}
