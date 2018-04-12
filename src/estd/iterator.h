#pragma once

#include "../platform.h"

namespace estd {

template< class InputIt >
typename std::iterator_traits<InputIt>::difference_type
    distance( InputIt first, InputIt last );

}
