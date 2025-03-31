/**
 * @file
 * standalone implementation so that can be #included from support_platform.h
 */
#pragma once

#include "cstddef.h"
#include "../fwd/type_traits.h"

namespace estd {

#if __cpp_alias_templates
template <class T>
struct type_identity { using type = T; };

template <class T>
using type_identity_t = typename type_identity<T>::type;
#else
template <class T>
struct type_identity { typedef T type; };
#endif


template<class T, T v>
struct integral_constant
{
    static constexpr T value = v;
    typedef T value_type;
    using type = integral_constant; // using injected-class-name

    ESTD_CPP_CONSTEXPR_RET operator value_type() const NOEXCEPT { return value; }
#ifdef __cpp_constexpr
    constexpr value_type operator()() const noexcept { return value; } //since c++14
#endif
};

// After c++11 this is deprecated, c++ and before it's (technically) required
// 31MAR25 MB DEBT: It seems c++14 needs it
#if __cplusplus <= 201402L
template<class T, T v>
CONSTEXPR T integral_constant<T, v>::value;
#endif


#ifdef __cpp_alias_templates
template <bool B>
using bool_constant = integral_constant<bool, B>;
#endif


typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

template< class T > struct remove_const          { typedef T type; };
template< class T > struct remove_const<const T> { typedef T type; };

template< class T > struct remove_volatile             { typedef T type; };
template< class T > struct remove_volatile<volatile T> { typedef T type; };


template< class T > struct remove_reference      { typedef T type; };
template< class T > struct remove_reference<T&>  { typedef T type; };
#ifdef __cpp_rvalue_references
template< class T > struct remove_reference<T&&> { typedef T type; };
#endif

#ifdef __cpp_alias_templates
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

template <class T>
struct add_lvalue_reference : type_identity<T&> {};

template <>
struct add_lvalue_reference<void> : type_identity<void> {};

#ifdef __cpp_rvalue_references
template <class T>
struct add_rvalue_reference : type_identity<T&&> {};;
#endif

#ifdef __cpp_alias_templates
template <class T>
using add_const_t = typename add_const<T>::type;

template <class T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

template <class T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;
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

template<class T> struct is_volatile             : false_type {};
template<class T> struct is_volatile<volatile T> : true_type {};

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


template <class T> struct remove_pointer { typedef T type; };
template <class T> struct remove_pointer<T*> { typedef T type; };
template <class T> struct remove_pointer<T* const> { typedef T type; };
template <class T> struct remove_pointer<T* volatile> { typedef T type; };
template <class T> struct remove_pointer<T* const volatile> { typedef T type; };

#if __cpp_alias_templates
template <class T>
using remove_pointer_t = typename remove_pointer<T>::type;
#endif


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

#pragma push_macro("B1")
#undef B1


#if __cpp_variadic_templates
// Shamelessly lifted from https://en.cppreference.com/w/cpp/types/conjunction
template<class...> struct conjunction : true_type {};

template<class B1> struct conjunction<B1> : B1 {};
template<class B1, class... Bn>
struct conjunction<B1, Bn...>
    : conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

template<class...> struct disjunction : false_type {};
template<class B1> struct disjunction<B1> : B1 {};
template<class B1, class... Bn>
struct disjunction<B1, Bn...>
    : conditional_t<bool(B1::value), B1, disjunction<Bn...>>  {};

#if __cpp_inline_variables
template< class... B >
inline constexpr bool conjunction_v = conjunction<B...>::value;

template <class T, class U>
inline constexpr bool is_same_v = is_same<T, U>::value;
#endif
#endif

#pragma pop_macro("B1")

}
