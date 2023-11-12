#pragma once

#include "tuple-shared.h"

namespace estd {

#if __cpp_variadic_templates
namespace internal {

template <class...>
class tuple;

}

template <class... Args>
using tuple = internal::tuple<Args...>;

template <std::size_t I, class T>
using tuple_element_t = typename tuple_element<I, T>::type;
#endif

}
