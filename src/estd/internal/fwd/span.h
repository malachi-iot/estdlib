#pragma once

#include "../raw/type_traits.h"

#if FEATURE_STD_SPAN
#include <span>
#endif

namespace estd {

namespace detail {

using dynamic_extent = integral_constant<size_t, (size_t)-1>;

template <class T>
struct is_span : false_type {};

}

template <class T, size_t Extent = detail::dynamic_extent()>
class span;

#if __cpp_inline_variables
inline constexpr size_t dynamic_extent = detail::dynamic_extent::value;
#endif

namespace detail {

template <class T, size_t N>
struct is_span<estd::span<T, N>> : true_type
{
    static constexpr bool std_span = false;
};

#if FEATURE_STD_SPAN
template <class T, size_t N>
struct is_span<std::span<T, N>> : true_type
{
    static constexpr bool std_span = true;
};
#endif

}

}
