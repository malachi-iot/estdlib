#pragma once

#include "internal/platform.h"
#ifdef FEATURE_STD_ITERATOR
#include <iterator>

namespace estd {

template< class InputIt >
//typename std::iterator_traits<InputIt>::difference_type
    int
    distance( InputIt first, InputIt last )
{
    int count = 0;
    while(first != last)
    {
        count++;
        first++;
    }
    //std::iterator_traits<InputIt>::difference_type count = 0;

    return count;
}

}
#else
namespace std {

// picked up from LLVM.  Not really relied on by our own libs at this time
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag       : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

}

#endif
