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

}