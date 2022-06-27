#pragma once

#include "../../cstddef.h"

namespace estd {

template <class T>
struct tuple_size;

template<std::size_t I, class T>
struct tuple_element;


#ifdef FEATURE_CPP_VARIADIC
template<class... TArgs>
class tuple;
#endif

}