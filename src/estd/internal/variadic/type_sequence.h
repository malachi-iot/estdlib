#pragma once

#include "fwd.h"
#include "../raw/type_traits.h"

#if __cpp_variadic_templates
namespace estd { namespace internal {

template <class T, class ...Types>
struct get_type_at_index<0, T, Types...> : type_identity<T>
{
};


template <size_t pos, class T, class ...Types>
struct get_type_at_index<pos, T, Types...> :
        get_type_at_index<pos - 1, Types...>
{
};

// DEBT: Try to consolidate with "get_index_finder" and "selector"

// defaults to zero matches (value == 0)
template <class Matching, class ...Types>
struct detail_get_index_of_type : integral_constant<int, 0> { };

// keeps looking during no match
template <class Matching, class T, class ...Types>
struct detail_get_index_of_type<Matching, T, Types...> : detail_get_index_of_type<Matching, Types...> {};

// increments match counter (++value), finds first occurence of Match but continues
template <class Matched, class ...Types>
struct detail_get_index_of_type<Matched, Matched, Types...> :
    integral_constant<int, detail_get_index_of_type<Matched, Types...>::value + 1>
{
    static constexpr unsigned index = sizeof ...(Types);
};

template <class Match, class ...Types>
struct first_index_of_type
{
    using detail = detail_get_index_of_type<Match, Types...>;

    static constexpr unsigned matches = detail::value;
    static constexpr unsigned index = (sizeof...(Types) - 1) - detail::index;
};

template <class Match, class ...Types>
struct single_index_of_type : first_index_of_type<Match, Types...>
{
    static_assert(first_index_of_type<Match, Types...>::matches == 1, "One and only one match is permitted");
};

template <class ...Types>
struct type_sequence_accessor
{
    static constexpr size_t size() { return sizeof...(Types); }

    template <size_t pos>
    using get = type_at_index<pos, Types...>;

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


// DEBT: Filter this out by concept/enabled
// DEBT: We'd like this for integer_sequence too
template <bool B, class Sequence, class T>
using prepend_if = conditional_t<B,
    typename Sequence::template prepend<T>,
    Sequence>;

template <bool B, class Sequence, typename Sequence::value_type v>
using value_prepend_if = conditional_t<B,
    typename Sequence::template prepend<v>,
    Sequence>;

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

    using visitor = variadic::type_visitor<Types...>;

    static constexpr bool empty() { return sizeof...(Types) == 0; }

    // +++ EXPERIMENTAL, not sure I want to put *this much* into type sequence
    template <class Eval>
    using selector = variadic::v2::selector<Eval, Types...>;

    template <class Eval>
    using where = typename selector<Eval>::types;
    // ---
};

}

}
#else
namespace estd { namespace internal {

template <class T1, class T2, class T3>
struct get_type_at_index<0, T1, T2, T3> : type_identity<T1> {};

template <class T1, class T2, class T3>
struct get_type_at_index<1, T1, T2, T3> : type_identity<T2> {};

template <class T1, class T2, class T3>
struct get_type_at_index<2, T1, T2, T3> : type_identity<T3> {};

}}
#endif
