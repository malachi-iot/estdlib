#pragma once

#include "internal/platform.h"
#include "internal/heap.h"
#include "internal/functional.h"
#include "internal/feature/algorithm.h"
#include "internal/algorithm/raw.h"
#include "internal/algorithm/heap.h"
#include "utility.h"

#include "internal/macro/push.h"

#ifdef FEATURE_STD_ALGORITHM
#include <algorithm>
#else
#undef FEATURE_ESTD_ALGORITHM_OPT
#endif

namespace estd {

// NOTE: Aliasing these out as best we can because std copy/fill operations may have some nice
// optimizations depending on your environment

// Shamelessly lifted from https://en.cppreference.com/w/cpp/algorithm/fill_n
template<class OutputIt, class Size, class T>
inline OutputIt fill_n(OutputIt first, Size count, const T& value)
{
#if FEATURE_ESTD_ALGORITHM_OPT
    return std::fill_n(first, count, value);
#else
    for (; count != 0; --count)
    {
        *first++ = value;
    }
    return first;
#endif
}

template< class ForwardIt, class T >
inline void fill(ForwardIt first, ForwardIt last, const T& value)
{
#if FEATURE_ESTD_ALGORITHM_OPT
    std::fill(first, last, value);
#else
    for (; first != last; ++first)
    {
        *first = value;
    }
#endif
}

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

template<class ForwardIt, class Compare>
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
ForwardIt min_element(ForwardIt first, ForwardIt last,
                      Compare comp)
{
    if (first == last) return last;

    ForwardIt smallest = first;
    ++first;
    for (; first != last; ++first) {
        if (comp(*first, *smallest)) {
            smallest = first;
        }
    }
    return smallest;
}



// DEBT: use overall macro push/pop here
#ifdef ESTD_MIN_SAVER
#define min ESTD_MIN_SAVER
#endif

template<class InputIt, class OutputIt>
inline OutputIt copy(InputIt first, InputIt last,
              OutputIt d_first)
{
#if FEATURE_ESTD_ALGORITHM_OPT
    return std::copy(first, last, d_first);
#else
    while (first != last) {
        *d_first++ = *first++;
    }
    return d_first;
#endif
}

// https://en.cppreference.com/w/cpp/algorithm/copy_n
// has a more complex implementation, but unsure why.  Maybe they want to avoid incrementing the source
// iterator unnecessarily?
template <class InputIt, class Size, class OutputIt>
inline OutputIt copy_n(InputIt first, Size count, OutputIt result)
{
    while(count--)
        *result++ = *first++;

    return result;
}


template<class InputIt, class OutputIt>
inline OutputIt copy_backward(InputIt first, InputIt last,
              OutputIt d_last)
{
#if FEATURE_ESTD_ALGORITHM_OPT
    return std::copy_backward(first, last, d_last);
#else
    while (first != last)
        *--d_last = *--last;

    return d_last;
#endif
}

#if __cpp_rvalue_references
template<class InputIt, class OutputIt>
inline OutputIt move_backward(InputIt first, InputIt last,
              OutputIt d_last)
{
#if FEATURE_ESTD_ALGORITHM_OPT
    return std::move_backward(first, last, d_last);
#else
    while (first != last)
        *--d_last = std::move(*--last);

    return d_last;
#endif
}
#endif


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

template<class T>
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
const T& clamp( const T& v, const T& lo, const T& hi )
{
    return clamp( v, lo, hi, estd::less<T>() );
}


template<class T, class Compare>
#if defined(FEATURE_CPP_CONSTEXPR_METHOD) && !defined(ESP8266)
constexpr
#endif
const T& clamp( const T& v, const T& lo, const T& hi, Compare comp )
{
    return assert( !comp(hi, lo) ),
        comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

// UNTESTED
template<class InputIt1, class InputIt2>
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
estd::pair<InputIt1, InputIt2> mismatch(InputIt1 first1, InputIt1 last1, InputIt2 first2)
{
    while (first1 != last1 && *first1 == *first2) {
        ++first1, ++first2;
    }
    return estd::make_pair(first1, first2);
}

template<class ForwardIt1, class ForwardIt2>
#if __cpp_constexpr >= 201304
constexpr
#endif
void iter_swap(ForwardIt1 a, ForwardIt2 b)
{
    swap(*a, *b);
}

template<class BidirIt>
#if __cpp_constexpr >= 201304
constexpr
#endif
void reverse(BidirIt first, BidirIt last)
{
    while ((first != last) && (first != --last))
        iter_swap(first++, last);
}

}

#include "internal/macro/pop.h"
