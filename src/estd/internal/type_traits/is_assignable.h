#pragma once

#include "features.h"

#include "../utility.h"

#if FEATURE_ESTD_TYPE_TRAITS_ALIASED
#include <type_traits>

namespace estd {

template <class T, class U>
using is_assignable = std::is_assignable<T, U>;

template <class T>
using is_copy_assignable = std::is_copy_assignable<T>;

template <class T>
using is_move_assignable = std::is_move_assignable<T>;

template <class T>
using is_nothrow_move_assignable = std::is_nothrow_move_assignable<T>;


}

#elif __cpp_alias_templates

namespace estd {

template<typename T, typename U, typename = void>
struct is_assignable : false_type {};

template<typename T, typename U>
struct is_assignable<T, U, decltype(std::declval<T>() = std::declval<U>(), void())> :
    true_type {};

// Shamelessly lifted from https://en.cppreference.com/w/cpp/types/is_copy_assignable

template<class T>
struct is_copy_assignable :
    is_assignable<typename add_lvalue_reference<T>::type,
        typename add_lvalue_reference<const T>::type> {};

/*
template<class T>
struct is_trivially_copy_assignable :
    is_trivially_assignable<typename add_lvalue_reference<T>::type,
        typename add_lvalue_reference<const T>::type> {};

template<class T>
struct is_nothrow_copy_assignable :
    is_nothrow_assignable<typename add_lvalue_reference<T>::type,
        typename add_lvalue_reference<const T>::type> {};
*/

// Shamelessly lifted from https://en.cppreference.com/w/cpp/types/is_move_assignable

template<class T>
struct is_move_assignable :
    is_assignable<add_lvalue_reference_t<T>,
        add_rvalue_reference_t<T>> {};

/*
template<class T>
struct is_trivially_move_assignable
    : is_trivially_assignable<typename add_lvalue_reference<T>::type,
                                   typename add_rvalue_reference<T>::type> {};

template<class T>
struct is_nothrow_move_assignable
    : is_nothrow_assignable<typename add_lvalue_reference<T>::type,
                                 typename add_rvalue_reference<T>::type> {};
*/

}

#endif
