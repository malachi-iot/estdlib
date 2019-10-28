/*
 *  @file
 *
 * Portion of type traits with no dependencies (except internal/platform.h)
 */

#pragma once

#include "platform.h"

namespace estd {

template <class T> struct remove_cv;
template <class T> struct is_integral;
template <class T> struct is_floating_point;
template <class T> struct is_pointer;
template <class T> struct is_reference;
template <class T> struct is_member_pointer;
template <class T> struct is_union;
template <class T> struct is_class;


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


template< class T > struct remove_const          { typedef T type; };
template< class T > struct remove_const<const T> { typedef T type; };

template< class T > struct remove_volatile             { typedef T type; };
template< class T > struct remove_volatile<volatile T> { typedef T type; };


template< class T > struct remove_reference      { typedef T type; };
template< class T > struct remove_reference<T&>  { typedef T type; };
#ifdef FEATURE_CPP_MOVESEMANTIC
template< class T > struct remove_reference<T&&> { typedef T type; };
#endif

#ifdef FEATURE_CPP_ALIASTEMPLATE
template< class T >
using remove_reference_t = typename remove_reference<T>::type;
#endif

template< class T >
struct remove_cv {
    typedef typename estd::remove_volatile<typename remove_const<T>::type>::type type;
};


template< class T >
struct add_cv { typedef const volatile T type; };

template< class T> struct add_const { typedef const T type; };

template< class T> struct add_volatile { typedef volatile T type; };

template<class T> struct is_const          : false_type {};
template<class T> struct is_const<const T> : true_type {};

template <class T> struct is_reference      : false_type {};
template <class T> struct is_reference<T&>  : true_type {};
#ifdef FEATURE_CPP_MOVESEMANTIC
template <class T> struct is_reference<T&&> : true_type {};
#endif

template<class T> struct is_lvalue_reference     : false_type {};
template<class T> struct is_lvalue_reference<T&> : true_type {};

#ifdef FEATURE_CPP_MOVESEMANTIC
template <class T> struct is_rvalue_reference      : false_type {};
template <class T> struct is_rvalue_reference<T&&> : true_type {};
#endif


}
