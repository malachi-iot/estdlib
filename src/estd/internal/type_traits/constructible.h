#pragma once

#include "features.h"

#if FEATURE_ESTD_TYPE_TRAITS_ALIASED
#include <type_traits>

namespace estd {

// DEBT: Copy/paste inline version of this from LLVM, GNU, etc.
#if __cpp_alias_templates
template <class T, class ...TArgs>
using is_constructible = std::is_constructible<T, TArgs...>;

template <class T>
using is_copy_constructible = std::is_copy_constructible<T>;

template <class T, class ...TArgs>
using is_nothrow_constructible = std::is_nothrow_constructible<T, TArgs...>;

template <class T>
using is_nothrow_move_constructible = std::is_nothrow_move_constructible<T>;

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
#elif __cpp_alias_templates

#include "raw/type_traits.h"
#include "utility/declval.h"

namespace estd {

// Shamelessly lifted from:
// https://stackoverflow.com/questions/38181357/how-is-stdis-constructiblet-args-implemented
// https://stackoverflow.com/questions/31207284/possible-implementation-of-is-assignable

namespace detail {

template <class, class T, class... Args>
struct is_constructible : false_type {};

template <class T, class... Args>
struct is_constructible<
    void_t<decltype(T(std::declval<Args>()...))>,
    T, Args...> : true_type {};

// 27JUN23 MB My own hacks of is_constructible
template <class, class T>
struct is_copy_constructible : false_type {};

template <class T>
struct is_copy_constructible<
    void_t<decltype(T(std::declval<const T&>()))>, T> : true_type {};

template <class, class T>
struct is_move_constructible : false_type {};

template <class T>
struct is_move_constructible<
    void_t<decltype(T(std::declval<T&&>()))>, T> : true_type {};
}

template <class T, class... Args>
using is_constructible = detail::is_constructible<void_t<>, T, Args...>;

template <class T>
using is_copy_constructible = detail::is_copy_constructible<void_t<>, T>;

// DEBT: Since exceptions are usually off, we cheat and reuse regular
// is_constructible.  Certainly not the right way to do things
template <class T, class... Args>
using is_nothrow_constructible = detail::is_constructible<void_t<>, T, Args...>;

template <class T, class... Args>
using is_nothrow_move_constructible = detail::is_move_constructible<void_t<>, T, Args...>;

}
#endif
