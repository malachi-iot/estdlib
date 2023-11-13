#pragma once

#include "tuple-shared.h"

namespace estd {

#if __cpp_variadic_templates
namespace internal {

template <bool sparse, class...>
class tuple;

}

template <class... Args>
using tuple = internal::tuple<true, Args...>;

inline namespace v1 {

template <class... Args>
using sparse_tuple = internal::tuple<true, Args...>;

}

template <std::size_t I, class T>
using tuple_element_t = typename tuple_element<I, T>::type;
#endif

}
