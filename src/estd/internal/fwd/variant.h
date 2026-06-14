#pragma once

#include "../variadic/fwd.h"
#include "../raw/variant.h"

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

template <bool trivial, class ...Args>
union variant_union;


template <bool trivial, class ...ypes>
struct variant_storage_base;

template <class ...Types>
class variant;

struct variant_storage_tag {};

template <class ...T>
using variant_storage = variant_storage_base<are_trivial<T...>::value, T...>;

template <int index, class ...Types>
inline ESTD_CPP_CONSTEXPR(14) type_at_index<index, Types...>* get_ll(variant<Types...>& vs) noexcept;

template <int index, class ...Types>
constexpr const type_at_index<index, Types...>* get_ll(const variant<Types...>& vs) noexcept;

template <class Visitor, class ...Types,
    class R = decltype(std::declval<Visitor>()(std::declval<type_at_index<0, Types...>>()))>
constexpr R visit(Visitor&&, variant<Types...>& variant);

template <class Visitor, class ...Types,
    class R = decltype(std::declval<Visitor>()(std::declval<type_at_index<0, Types...>>()))>
constexpr R visit(Visitor&&, const variant<Types...>& variant);

}

template <class ...Types>
using variant = internal::variant<Types...>;

#if __cplusplus >= 201703L
template <class Visitor, class ...Variants>
constexpr auto visit(Visitor&&, Variants&&...);
#endif

}
