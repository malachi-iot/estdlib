#pragma once

#include "is_function.h"

// Shamelessly lifted from:
// https://en.cppreference.com/w/cpp/types/is_member_function_pointer
// https://en.cppreference.com/w/cpp/types/is_member_pointer
// https://en.cppreference.com/w/cpp/types/is_member_object_pointer

// UNTESTED

namespace estd {

namespace internal {

template <class T>
struct is_member_function_pointer : false_type {};
 
template <class T, class U>
struct is_member_function_pointer<T U::*> : is_function<T> {};

template<class T>
struct is_member_pointer : false_type {};
 
template<class T, class U>
struct is_member_pointer<T U::*> : true_type {};
 
}

template<class T>
struct is_member_pointer :
    internal::is_member_pointer<typename remove_cv<T>::type> {};

template <class T>
using is_member_function_pointer = 
    internal::is_member_function_pointer<typename remove_cv<T>::type>;

template<class T>
struct is_member_object_pointer : integral_constant<
    bool,
    is_member_pointer<T>::value &&
    !is_member_function_pointer<T>::value> {};

#if __cpp_inline_variables
template< class T >
inline constexpr bool is_member_pointer_v = is_member_pointer<T>::value;

template <class T>
inline constexpr bool is_member_function_pointer_v =
    is_member_function_pointer<T>::value;
    
template< class T >
inline constexpr bool is_member_object_pointer_v = is_member_object_pointer<T>::value;
#endif

}