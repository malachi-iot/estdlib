#pragma once

#include "fwd.h"
#include "../raw/type_traits.h"

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
    static constexpr size_t size() { return sizeof...(Types); }

    template <class T>
    using prepend = type_sequence<T, Types...>;

    template <class T>
    using append = type_sequence<Types..., T>;

    // DEBT: Not yet doing typename ::type treatment here because it seems 'expected' code
    // somehow tries to grab pos=0 from an empty Types...
    template <size_t pos>
    using get = get_type_finder<pos, Types...>;

    // DEBT: Temporary until above is resolved
    template <size_t pos>
    using get_t = typename get_type_finder<pos, Types...>::type;

    using first = get<0>;

    using last = get<size() - 1>;
};


// Very similar to std::variant_alternative
template <size_t index, class ...Types>
using type_at_index = typename get_type_finder<index, Types...>::type;


}}
#endif
