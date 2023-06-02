#pragma once

#include "../raw/cstddef.h"
#include "../raw/utility.h"
#include "../raw/variant.h"

#if __cpp_variadic_templates
namespace estd { namespace internal {

template <size_t pos, class ...Types>
struct get_type_finder;

template <class TEval, class ...Types>
struct visitor_helper_struct;

template <unsigned size, class TEval, class ...Types>
struct visitor_helper_struct2;

template <class ...Types>
struct variadic_first;

// Plural of is_trivial
// DEBT: Consider putting out into main estd namespace
template <class ...TArgs>
struct are_trivial;

}}
#endif
