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
struct type_sequence_accessor
{
    static constexpr size_t size() { return sizeof...(Types); }

    template <size_t pos>
    using get = typename get_type_finder<pos, Types...>::type;

    using first = get<0>;

    using last = get<size() - 1>;
};

template <>
struct type_sequence_accessor<>
{
    static constexpr size_t size() { return 0; }
};

template <class ...Types>
struct type_sequence_single {};

template <class T>
struct type_sequence_single<T>
{
    using single = T;
};


// Very similar to std::variant_alternative
template <size_t index, class ...Types>
using type_at_index = typename get_type_finder<index, Types...>::type;

// DEBT: Filter this out by concept/enabled
// DEBT: We'd like this for integer_sequence too
template <bool B, class TSequence, class T>
using prepend_if = conditional_t<B,
    typename TSequence::template prepend<T>,
    TSequence>;


}

namespace variadic {

template <class ...Types>
struct types :
        internal::type_sequence_accessor<Types...>,
        internal::type_sequence_single<Types...>
{
    template <class T>
    using prepend = types<T, Types...>;

    template <class T>
    using append = types<Types..., T>;

    static constexpr bool empty() { return sizeof...(Types) == 0; }
};

}

}
#endif
