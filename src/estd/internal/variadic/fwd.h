#pragma once

#include "../raw/cstddef.h"
#include "../raw/utility.h"

#if __cpp_variadic_templates
namespace estd { namespace internal {

template <size_t pos, class ...Types>
struct get_type_at_index;

// Very similar to std::variant_alternative
template <size_t index, class ...Types>
using type_at_index = typename get_type_at_index<index, Types...>::type;

// Plural of is_trivial
// DEBT: Consider putting out into main estd namespace
template <class ...TArgs>
struct are_trivial;

// Indicates the function/constructor expects a functor for iterating
// over all the variadic possibilities
struct in_place_visit_t : in_place_tag {};

// internal = specialization version, since std indicates it's undefined to specialize conjunction
template <class T>
struct conjunction;

}

namespace variadic {

namespace detail {

template <size_t size, class TEval, class ...Types>
struct selector;

}

template <class TEval, class ...Types>
using selector = typename detail::selector<sizeof...(Types), TEval, Types...>::selected;

template <class TEval, class ...Types>
using projector = typename detail::selector<sizeof...(Types), TEval, Types...>::projected;

template <size_t I, class T>
struct visitor_index;

template <class ...Types>
struct types;

template <typename T, T ...Is>
struct values;

template <class T, bool v = true>
struct projected_result;

}

template <class ...Types>
using type_sequence = variadic::types<Types...>;

}
#else
namespace estd {

namespace internal {

template <size_t index, class T1, class T2, class T3>
struct get_type_at_index;

template <class T1, class T2, class T3>
struct are_trivial;

}

}
#endif
