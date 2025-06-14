#pragma once

#include "../deduce_fixed_size.h"

namespace estd {

namespace internal {

// DEBT: A concept here would be nice, though we don't anticipate anyone other than
// traditional_array and uninitialized_array as impl
template <class Impl>
struct array;

// Deprecated name
template <class Base>
using array_base2 = array<Base>;

namespace impl {

template <class T, size_t N>
struct traditional_array;

}

}

template<class T, size_t N>
using array = internal::array<internal::impl::traditional_array<T, N> >;


}
