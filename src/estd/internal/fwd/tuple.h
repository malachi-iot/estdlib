#pragma once

#include "../feature/tuple.h"

#include "tuple-shared.h"

namespace estd {

#if __cpp_variadic_templates
namespace internal {

template <bool sparse, class T, std::size_t index = 0, class enabled = void>
class tuple_storage;

template <bool sparse, class...>
class tuple;

}

inline namespace v1 {

template <class... Args>
using sparse_tuple = internal::tuple<true, Args...>;

}

template <class... Args>
#if FEATURE_ESTD_IS_EMPTY && FEATURE_ESTD_SPARSE_TUPLE
using tuple = v1::sparse_tuple<Args...>;
#else
using tuple = internal::tuple<false, Args...>;
#endif


template <std::size_t I, class T>
using tuple_element_t = typename tuple_element<I, T>::type;
#endif

}
