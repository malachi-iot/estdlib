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

// Arduino lib makes its own min
#ifdef min
#define ESTD_MIN_SAVER min
#undef min
#endif

// https://en.cppreference.com/w/cpp/algorithm/min
template<class T>
const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

#ifdef ESTD_MIN_SAVER
#define min ESTD_MIN_SAVER
#endif

template<class InputIt, class OutputIt>
OutputIt copy(InputIt first, InputIt last,
              OutputIt d_first)
{
    while (first != last) {
        *d_first++ = *first++;
    }
    return d_first;
}

template<class InputIt, class UnaryPredicate>
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
InputIt find_if(InputIt first, InputIt last, UnaryPredicate p)
{
    for (; first != last; ++first) {
        if (p(*first)) {
            return first;
        }
    }
    return last;
}

}
