#pragma once

#include "internal/platform.h"
#include <iterator>

namespace estd {

template< class InputIt >
typename std::iterator_traits<InputIt>::difference_type
    distance( InputIt first, InputIt last );

}
