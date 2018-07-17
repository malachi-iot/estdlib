#pragma once

#include "internal/platform.h"

// mainly to fill in gaps for pre C++11 compatibility
namespace estd {

// https://en.cppreference.com/w/cpp/algorithm/equal
template<class InputIt1, class InputIt2>
bool equal(InputIt1 first1, InputIt1 last1,
           InputIt2 first2)
{
    for (; first1 != last1; ++first1, ++first2) {
        if (!(*first1 == *first2)) {
            return false;
        }
    }
    return true;
}

// https://en.cppreference.com/w/cpp/algorithm/min
template<class T>
const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}


}
