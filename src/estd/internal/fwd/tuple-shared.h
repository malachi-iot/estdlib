// Shared by both <= c++03 and > c++03 headers
#pragma once

#include "../../cstddef.h"

namespace estd {

template <class T>
struct tuple_size;

template<std::size_t I, class T>
struct tuple_element;

}