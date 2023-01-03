#pragma once

#include "../deduce_fixed_size.h"

namespace estd {

template <class T, std::size_t N, typename TSize = typename internal::deduce_fixed_size_t<N>::size_type>
struct array;

}