#pragma once

#include "../deduce_fixed_size.h"

namespace estd {

namespace internal {

template <class Base>
struct array_base2;

namespace impl {

template <class T, unsigned N>
struct traditional_array;

}

}

template<class T, std::size_t N, typename Size = typename internal::deduce_fixed_size_t<N>::size_type>
using array = internal::array_base2<internal::impl::traditional_array<T, N> >;


}
