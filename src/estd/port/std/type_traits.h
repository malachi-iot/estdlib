#pragma once

#include <type_traits>

namespace std {

// NOLINTBEGIN

// Polyfill
#if __cplusplus <= 201103L
template <class ...Args>
using common_type_t = typename common_type<Args...>::type;
#endif

// NOLINTEND

}
