#pragma once

#include "../deduce_fixed_size.h"

namespace estd {

namespace internal {

// DEBT: Deprecated name
template <class Base>
struct array_base2;

// DEBT: A concept here would be nice, though we don't anticipate anyone other than
// traditional_array and uninitialized_array as impl
template <class Impl>
using array = array_base2<Impl>;

namespace impl {

template <class T, unsigned N>
struct traditional_array;

}

}

template<class T, std::size_t N, typename Size = typename internal::deduce_fixed_size_t<N>::size_type>
using array = internal::array<internal::impl::traditional_array<T, N> >;


}
