// Just 'is_empty' portion of type_traits, to reduce dependency footprint
#pragma once

#include "../../port/is_empty.h"

namespace estd {

#if defined(FEATURE_CPP_CONSTEXPR) && FEATURE_ESTD_IS_EMPTY
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
