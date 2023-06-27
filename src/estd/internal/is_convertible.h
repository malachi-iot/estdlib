#pragma once

#include "platform.h"
#include "raw/type_traits.h"
#include "../utility.h"
#include "utility/declval.h"

// Specifically for is_void
#include "llvm/is_void.h"

#if __cplusplus >= 201103L

// DEBT: Only pulling in this for declval.  Not pulling in utility.h since that itself
// includes type_traits which is responsible for is_convertible.  Consider our own implementation
// of std::declval as per https://en.cppreference.com/w/cpp/utility/declval
#ifdef FEATURE_STD_UTILITY
#include <utility>
#endif

// Shamelessly lifted from https://en.cppreference.com/w/cpp/types/is_convertible
namespace estd {
namespace detail {
 
template<class T>
auto test_returnable(int) -> decltype(
    void(static_cast<T(*)()>(nullptr)), true_type{}
);
template<class>
auto test_returnable(...) -> false_type;
 
template<class From, class To>
auto test_implicitly_convertible(int) -> decltype(
    void(std::declval<void(&)(To)>()(std::declval<From>())), true_type{}
);
template<class, class>
auto test_implicitly_convertible(...) -> false_type;
 
} // namespace detail
 
template<class From, class To>
struct is_convertible : integral_constant<bool,
    (decltype(detail::test_returnable<To>(0))::value &&
     decltype(detail::test_implicitly_convertible<From, To>(0))::value) ||
    (is_void<From>::value && is_void<To>::value)
> {};

#if __cpp_inline_variables
template< class From, class To >
inline constexpr bool is_convertible_v = is_convertible<From, To>::value;
#endif

}

#endif