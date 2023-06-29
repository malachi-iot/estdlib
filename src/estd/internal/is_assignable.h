#pragma once

#include "platform.h"

#include "feature/type_traits.h"
#include "raw/type_traits.h"

#include "utility.h"

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

template<typename T, typename U, typename = void>
struct is_copy_assignable : false_type {};

template<typename T, typename U, typename = void>
struct is_move_assignable : false_type {};

}

#endif