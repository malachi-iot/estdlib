// Shared by both <= c++03 and > c++03 headers
#pragma once

#include "../../cstddef.h"

namespace estd {

namespace internal {

template <bool sparse, class T, size_t index = 0, class enabled = void>
class tuple_storage;

}

template <class T>
struct tuple_size;

template<std::size_t I, class T>
struct tuple_element;

}