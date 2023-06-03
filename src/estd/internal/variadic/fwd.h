#pragma once

#include "../raw/cstddef.h"
#include "../raw/utility.h"
#include "../raw/variant.h"

#if __cpp_variadic_templates
namespace estd { namespace internal {

template <class ...Types>
struct type_sequence;

template <size_t pos, class ...Types>
struct get_type_finder;

template <size_t size, class TEval, class ...Types>
struct visitor_helper_struct2;

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

template <class TEval, class ...Types>
using selector = typename internal::visitor_helper_struct2<sizeof...(Types), TEval, Types...>::selected;

template <class TEval, class ...Types>
using projector = typename internal::visitor_helper_struct2<sizeof...(Types), TEval, Types...>::projected;

template <size_t I, class T>
struct visitor_index;


}

}
#endif
