#pragma once

#include "dynamic_array.h"
#include "../../policy/string.h"

namespace estd {

namespace internal {

template <class Allocator, class Policy>
class basic_string;

}

template<
    class CharT,
    class Traits,
    class Allocator,
    class Policy
> class basic_string;

}