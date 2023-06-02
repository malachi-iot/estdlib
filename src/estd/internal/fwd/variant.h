#pragma once

#include "../variadic/fwd.h"

namespace estd {

namespace internal {

template <bool trivial, class ...TArgs>
union variant_union;


template <bool trivial, class ...Types>
struct variant_storage_base;

template <class ...Types>
class variant;

struct variant_storage_tag {};

template <class ...T>
using variant_storage = variant_storage_base<are_trivial<T...>::value, T...>;

template <class T>
struct variant_size;

template <unsigned I, class T>
struct variant_alternative;

template <unsigned I, class T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

}

}
