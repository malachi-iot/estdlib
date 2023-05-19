// Just 'is_empty' portion of type_traits, to reduce dependency footprint
#pragma once

#include "platform.h"
#include "raw/type_traits.h"

namespace estd {

// Duplicating code because __has_extension is not available in GCC
#ifdef __llvm__
// http://releases.llvm.org/3.5.1/tools/clang/docs/LanguageExtensions.html
#if __has_extension(is_empty)
#define FEATURE_ESTD_IS_EMPTY 1
template<typename _Tp>
struct is_empty
        : public integral_constant<bool, __is_empty(_Tp)>
{};
#endif
#elif defined(__GNUC__)
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __GNUC_PREREQ(4,3)
// https://stackoverflow.com/questions/35531309/how-is-stdis-emptyt-implemented-in-vs2015-or-any-compiler
// https://gcc.gnu.org/onlinedocs/gcc-4.5.4/gcc/Type-Traits.html
// https://www.boost.org/doc/libs/1_65_1/boost/type_traits/intrinsics.hpp
// /usr/include/c++/4.8/type_traits:516
#define FEATURE_ESTD_IS_EMPTY 1
template<typename _Tp>
struct is_empty
    : public integral_constant<bool, __is_empty(_Tp)>
{};
#endif  // GCC exp/4.3
#elif _MSC_VER
#define FEATURE_ESTD_IS_EMPTY 1
template<typename _Tp>
struct is_empty
    : public integral_constant<bool, __is_empty(_Tp)>
{};
#elif 0 // LLVM
#endif

#if defined(FEATURE_CPP_CONSTEXPR)
#if defined(FEATURE_CPP_INLINE_VARIABLES)
template <class T>
inline constexpr bool is_empty_v = is_empty<T>::value;
#endif
/// Non-standard function deviation of is_empty_v for pre-C++17 scenarios
/// Somewhat experimental
/// \tparam T
/// \return
template <class T>
inline constexpr bool is_empty_f() { return is_empty<T>::value; }
#endif

}
