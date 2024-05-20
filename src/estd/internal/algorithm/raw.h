#pragma once

// Similar to 'raw' folder paradigm, no #includes are needed here.  That's only an incidental
// behavior, but we like it.  Really, this is where some basics live instead of making a discrete fwd.h

#include "../macro/push.h"

namespace estd {

// https://en.cppreference.com/w/cpp/algorithm/min
template <class T>
#if __cpp_constexpr >= 201304L
constexpr
#endif
const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

template <class T, class Compare>
#if __cpp_constexpr >= 201304L
constexpr
#endif
const T& min(const T& a, const T& b, Compare comp)
{
    return (comp(b, a)) ? b : a;
}

template <class T>
#if __cpp_constexpr >= 201304L
constexpr
#endif
const T& max(const T& a, const T& b)
{
    return (b > a) ? b : a;
}


template<class ForwardIt, class Compare>
#if __cpp_constexpr >= 201304L
constexpr
#endif
ForwardIt max_element(ForwardIt first, ForwardIt last, Compare comp)
{
    if (first == last)
        return last;

    ForwardIt largest = first;
    ++first;

    for (; first != last; ++first)
        if (comp(*largest, *first))
            largest = first;

    return largest;
}
}

#include "../macro/pop.h"
