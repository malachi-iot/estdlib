#pragma once
// Non-standard type_traits-like tools for parameter pack

#include "platform.h"
#include "variadic/fwd.h"
#include "variadic/integer_sequence.h"
#include "variadic/type_sequence.h"
#include "variadic/visitor.h"

#if __cpp_variadic_templates

#include "../tuple.h"
#include "raw/type_traits.h"
#include "raw/variant.h"

namespace estd { namespace internal {

template <int ...Is>
struct indices_reverser;

template <>
struct indices_reverser<>
{
    using reversed = index_sequence<>;
};


template <int I, int ...Is>
struct indices_reverser<I, Is...>
{
    typedef indices_reverser<Is...> next;

    // NOTE: Not proven or tested yet
    using reversed = typename next::reversed::template append<I>;
};

//template <int pos, int ...Is>
//using get_index = get_index_finder<pos, sizeof...(Is), Is...>;



// EXPERIMENTAL, perhaps we prefer this specialization over functor?
template <class TProvider>
struct visitor_instance_factory
{
    template <unsigned I, class T>
    variadic::visitor_instance<I, T> create(variadic::visitor_index<I, T>) { return {}; }
};



// EXPERIMENTAL
template <class T>
struct converting_selector
{
    template <class T_j, size_t>
    using evaluator = is_convertible<T, T_j>;
};


// EXPERIMENTAL
template <class ...Types>
struct constructable_selector
{
    template <class T_j, size_t>
    using evaluator = is_constructible<T_j, Types...>;
};

struct is_copy_constructible_selector
{
    template <class T_j, size_t>
    using evaluator = is_copy_constructible<T_j>;
};


template <class T>
struct is_same_selector
{
    template <class T_j, size_t>
    using evaluator = is_same<T_j, T>;
};

/*
 * Actually works, but more complicated than it needs to be since we're
 * not really projecting
template <size_t I>
struct index_selector
{
    template <class T_j, size_t J>
    using evaluator = projected_result<I == J, T_j>;
};
*/
template <size_t I>
struct index_selector
{
    template <class, size_t J>
    using evaluator = bool_constant<I == J>;
};


// largest_type lifted from
// https://stackoverflow.com/questions/16803814/how-do-i-return-the-largest-type-in-a-list-of-types
template <typename... Ts>
struct largest_type;

template <typename T>
struct largest_type<T>
{
    using type = T;
};

template <typename T, typename U, typename... Ts>
struct largest_type<T, U, Ts...>
{
    using type = typename largest_type<typename estd::conditional<
            (sizeof(U) <= sizeof(T)), T, U>::type, Ts...
        >::type;
};

// Plural of is_trivial
// DEBT: Consider putting out into main estd namespace
template <>
struct are_trivial<> : bool_constant<true>
{
};

template <class T, class ...TArgs>
struct are_trivial<T, TArgs...>
{
    static constexpr bool value = estd::is_trivial<T>::value &
        are_trivial<TArgs...>::value;
};


template <class T, class ...TArgs>
using select_type = variadic::selector2<is_same_selector<T>, TArgs...>;

// Evaluate true or false whether a variadic contains a particular type
template <class T, class ...Types>
using contains_type = bool_constant<!select_type<T, Types...>::empty()>;

// Only enables if type T is contained in Types
template <class T, class ...Types>
using ensure_type = enable_if<contains_type<T, Types...>::value, T>;

template <class T, class ...Types>
using ensure_type_t = typename ensure_type<T, Types...>::type;

}

namespace variadic {

// Wrapper to help return specifically as an integral constant
template <class ...Types>
using size = integral_constant<size_t, sizeof...(Types)>;

// DEBT: Consider this should return the contained ::type, ala type_sequence
template <class T, class ...Types>
using first = type_identity<T>;

}

}
#endif
