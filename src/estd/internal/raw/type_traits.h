/**
 * @file
 * standalone implementation so that can be #included from support_platform.h
 */
#pragma once

#include "cstddef.h"
#include "../fwd/type_traits.h"

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

#ifdef __cpp_alias_templates
template <bool B>
using bool_constant = integral_constant<bool, B>;
#endif


typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

#ifdef FEATURE_CPP_ALIASTEMPLATE
template< class T >
struct type_identity { using type = T; };

template< class T >
using type_identity_t = typename type_identity<T>::type;
#else
template< class T >
struct type_identity { typedef T type; };
#endif

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
    typedef typename remove_volatile<typename remove_const<T>::type>::type type;
};

template< class T >
struct remove_cvref {
    typedef typename remove_cv<typename remove_reference<T>::type>::type type;
};

#ifdef __cpp_alias_templates
template< class T >
using remove_const_t = typename remove_const<T>::type;

template< class T >
using remove_volatile_t = typename remove_volatile<T>::type;

template< class T >
using remove_cv_t = typename remove_cv<T>::type;

template< class T >
using remove_cvref_t = typename remove_cvref<T>::type;
#endif

template< class T >
struct add_cv { typedef const volatile T type; };

template< class T> struct add_const { typedef const T type; };

#ifdef __cpp_alias_templates
template< class T >
using add_const_t = typename add_const<T>::type;
#endif

template< class T> struct add_volatile { typedef volatile T type; };

template<class T> struct is_const          : false_type {};
template<class T> struct is_const<const T> : true_type {};

template <class T> struct is_reference      : false_type {};
template <class T> struct is_reference<T&>  : true_type {};
#ifdef __cpp_rvalue_references
template <class T> struct is_reference<T&&> : true_type {};
#endif

template<class T> struct is_lvalue_reference     : false_type {};
template<class T> struct is_lvalue_reference<T&> : true_type {};

#ifdef FEATURE_CPP_MOVESEMANTIC
template <class T> struct is_rvalue_reference      : false_type {};
template <class T> struct is_rvalue_reference<T&&> : true_type {};
#endif

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


template<class T>
struct is_array : false_type {};

template<class T>
struct is_array<T[]> : true_type {};

template<class T, std::size_t N>
struct is_array<T[N]> : true_type {};

template<class T>
struct remove_extent { typedef T type; };

template<class T>
struct remove_extent<T[]> { typedef T type; };

template<class T, std::size_t N>
struct remove_extent<T[N]> { typedef T type; };


#ifdef FEATURE_CPP_ALIGN
// NOTE: deprecated in C++23
template<std::size_t Len, std::size_t Align /* default alignment not implemented */>
struct aligned_storage {
    struct type {
        alignas(Align) byte data[Len];
    };
};
#else
template<std::size_t Len, std::size_t Align>
struct aligned_storage {
    struct type {
        // FIX: Without alignas, we're gonna have problems.  Consider this experimental
        // FIX: Also gonna report wrong size from data, though that's rarely gonna be an issue
        byte data[Align];
    };
};
#endif

#ifdef __cpp_alias_templates
template<class...>
using void_t = void;

template< bool B, class T = void>
using enable_if_t = typename enable_if<B,T>::type;

template< bool B, class T, class F >
using conditional_t = typename conditional<B,T,F>::type;
#else
// UNTESTED
template< bool B, class T = void>
class enable_if_t : public enable_if<B, T>::type {};
#endif


#if __cpp_variadic_templates
// Shamelessly lifted from https://en.cppreference.com/w/cpp/types/conjunction
template<class...> struct conjunction : true_type {};

template<class B1> struct conjunction<B1> : B1 {};
template<class B1, class... Bn>
struct conjunction<B1, Bn...>
    : conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};
#endif

#if __cpp_inline_variables
template< class... B >
inline constexpr bool conjunction_v = conjunction<B...>::value;
#endif

}
