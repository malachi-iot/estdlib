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

template <unsigned size, class TEval, class ...Types>
struct visitor_helper_struct2;

// Plural of is_trivial
// DEBT: Consider putting out into main estd namespace
template <class ...TArgs>
struct are_trivial;

// Indicates the function/constructor expects a functor for iterating
// over all the variadic possibilities
struct in_place_visit_t : in_place_tag {};

}

namespace variadic {

template <class TEval, class ...Types>
struct selector;

template <size_t I, class T>
struct visitor_index;


}

}
#endif
