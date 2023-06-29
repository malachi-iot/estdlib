#pragma once

#include "common_type.h"

namespace estd {

template <class T> struct remove_cv;
template <class T> struct is_integral;
template <class T> struct is_floating_point;
template <class T> struct is_pointer;
template <class T> struct is_reference;
template <class T> struct is_member_pointer;
template <class T> struct is_union;
template <class T> struct is_class;

template< class T >
struct add_pointer;

#if __cpp_alias_templates
template <class T>
using add_pointer_t = typename add_pointer<T>::type;
#endif

template <class Base, class Derived>
struct is_base_of;


}