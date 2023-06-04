#pragma once

#include "../variadic/fwd.h"

namespace estd {

template <class T>
struct variant_size;

template <size_t I, class T>
struct variant_alternative;

template <size_t I, class T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

#if __cpp_exceptions
class bad_variant_access;
#endif

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

template <int index, class ...Types>
type_at_index<index, Types...>* get_ll(variant<Types...>& vs) noexcept;

template <int index, class ...Types>
constexpr const type_at_index<index, Types...>* get_ll(const variant<Types...>& vs) noexcept;

}

}
