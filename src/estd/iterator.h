#pragma once

#include "internal/platform.h"
#ifdef FEATURE_STD_ITERATOR
#include <iterator>

namespace estd {

template< class InputIt >
typename std::iterator_traits<InputIt>::difference_type
    distance( InputIt first, InputIt last );

}
#endif