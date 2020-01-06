#pragma once

#include "internal/platform.h"
#include "internal/functional.h"

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
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
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

// https://en.cppreference.com/w/cpp/algorithm/copy_n
// has a more complex implementation, but unsure why.  Maybe they want to avoid incrementing the source
// iterator unnecessarily?
template <class InputIt, class Size, class OutputIt>
OutputIt copy_n(InputIt first, Size count, OutputIt result)
{
    while(count--)
        *result++ = *first++;

    return result;
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

template<class T>
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
constexpr
#endif
const T& clamp( const T& v, const T& lo, const T& hi )
{
    return clamp( v, lo, hi, estd::less<T>() );
}


template<class T, class Compare>
#ifdef FEATURE_CPP_CONSTEXPR_METHOD
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

}
