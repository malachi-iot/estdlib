#pragma once

#include "../platform.h"

// mainly to fill in gaps where pre-C++03 is used
namespace estd {

template<class T, T v>
struct integral_constant {
    static CONSTEXPR T value = v;
    typedef T value_type;
    typedef integral_constant type; // using injected-class-name
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    operator value_type() const { return value; }
    //constexpr value_type operator()() const noexcept { return value; } //since c++14
};


typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

// lifted from http://en.cppreference.com/w/cpp/types/conditional
template<bool B, class T, class F>
struct conditional { typedef T type; };

template<class T, class F>
struct conditional<false, T, F> { typedef F type; };

// lifted from http://en.cppreference.com/w/cpp/types/is_same
template<class T, class U>
struct is_same : false_type {};

template<class T>
struct is_same<T, T> : true_type {};

// lifted from http://en.cppreference.com/w/cpp/types/enable_if
template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };

#ifdef FEATURE_CPP_ALIASTEMPLATE
template< bool B, class T = void >
using enable_if_t = typename enable_if<B,T>::type;
#else
// UNTESTED
template< bool B, class T = void >
class enable_if_t : public enable_if<B, T>::type {};
#endif

}
