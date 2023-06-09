#pragma once

#include "platform.h"

#if FEATURE_STD_TYPE_TRAITS
#include <type_traits>

namespace estd {

// DEBT: Copy/paste inline version of this from LLVM, GNU, etc.
#if __cpp_alias_templates
template <class T, class U>
using is_assignable = std::is_assignable<T, U>;

template <class T, class ...TArgs>
using is_constructible = std::is_constructible<T, TArgs...>;

template <class T>
using is_copy_assignable = std::is_copy_assignable<T>;

template <class T>
using is_copy_constructible = std::is_copy_constructible<T>;

template <class T, class ...TArgs>
using is_nothrow_constructible = std::is_nothrow_constructible<T, TArgs...>;

template <class T>
using is_nothrow_move_assignable = std::is_nothrow_move_assignable<T>;

template <class T>
using is_nothrow_move_constructible = std::is_nothrow_move_constructible<T>;

template <class T>
using is_move_assignable = std::is_move_assignable<T>;

template <class T, class ...TArgs>
using is_trivially_constructible = std::is_trivially_constructible<T, TArgs...>;

#if __cpp_inline_variables
template <class T, class ...TArgs>
inline constexpr bool is_constructible_v = is_constructible<T, TArgs...>::value;

template <class T>
inline constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;

template <class T, class ...TArgs>
inline constexpr bool is_trivially_constructible_v = is_trivially_constructible<T, TArgs...>::value;
#endif

#endif

}

#endif
