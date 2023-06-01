#pragma once

#include "fwd.h"

#if __cpp_variadic_templates
namespace estd { namespace internal {

template <class T, class ...Types>
struct get_type_finder<0, T, Types...> : type_identity<T>
{
};


template <size_t pos, class T, class ...Types>
struct get_type_finder<pos, T, Types...> :
        get_type_finder<pos - 1, Types...>
{
};


template <class ...Types>
struct type_sequence
{
    template <class T>
    using prepend = type_sequence<T, Types...>;

    template <class T>
    using append = type_sequence<Types..., T>;

    template <size_t pos>
    using get = get_type_finder<pos, Types...>;

    static constexpr size_t size() { return sizeof...(Types); }
};


// Very similar to std::variant_alternative
template <size_t index, class ...Types>
using type_at_index = typename get_type_finder<index, Types...>::type;


}}
#endif