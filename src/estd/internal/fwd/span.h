#pragma once

#include "../raw/type_traits.h"

namespace estd {

namespace detail {

using dynamic_extent = integral_constant<size_t, (size_t)-1>;

}

template <class T, size_t Extent = detail::dynamic_extent()>
class span;

#if __cpp_inline_variables
inline constexpr size_t dynamic_extent = detail::dynamic_extent::value;
#endif


}
