#pragma once

#include "fwd.h"
#include "type_sequence.h"
#include "integer_sequence.h"
#include "../type_traits/is_base_of.h"

#if __cpp_variadic_templates
namespace estd {

namespace internal {

struct projected_result_tag {};

template <class T>
struct converting_selector
{
    template <class T_j, size_t>
    using evaluator = is_convertible<T, T_j>;
};


template <class ...Types>
struct constructible_selector
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


template <template <class, class...> class T, class ...TArgs>
struct projector_selector
{
    template <class T_j, size_t>
    using evaluator = variadic::projected_result<T<T_j, TArgs...>>;
};

template <class T>
using is_same_projector = projector_selector<is_same, T>;


}

namespace variadic {

template <class T, bool v>
struct projected_result :
        type_identity<T>,
        internal::projected_result_tag
{
    constexpr static bool value = v;
};

namespace detail {

template <size_t size, class TEval>
struct selector<size, TEval>
{
    using indices = index_sequence<>;
    using types = variadic::types<>;
    using projected = variadic::types<>;

    using selected = variadic::types<>;
};


template <size_t size, class TEval, class T, class ...Types>
struct selector<size, TEval, T, Types...>
{
private:
    typedef selector<size, TEval, Types...> upward;

    static constexpr size_t index = ((size - 1) - sizeof...(Types));

    using evaluated = typename TEval::template evaluator<T, index>;
    static constexpr bool eval = evaluated::value;

    using projected_type = conditional_t<
        is_base_of<internal::projected_result_tag, evaluated>::value,
        typename evaluated::type, T>;

    using visitor_index = variadic::visitor_index<index, projected_type>;

    template <class TSequence, class T2>
    using prepend = internal::prepend_if<eval, TSequence, T2>;

public:
    using indices = conditional_t<eval,
        typename upward::indices::template prepend<index>,
        typename upward::indices>;

    /// Raw list of types in this sequence
    using types = prepend<typename upward::types, T>;
    /// Converted (via Eval) list of types in this sequence
    using projected = prepend<typename upward::projected, projected_type>;
    /// List of visitor_index specifying Type and explicit index per variadic item
    using selected = prepend<typename upward::selected, visitor_index>;

    static constexpr bool all = selected::size() == sizeof...(Types) + 1;
};

template <size_t size, class TEval, class T, T t, T ...Values>
struct value_selector<size, TEval, T, t, Values...>
{
private:
    typedef value_selector<size, TEval, T, Values...> upward;

public:
    static constexpr size_t index = ((size - 1) - sizeof...(Values));

    using evaluated = typename TEval::template evaluator<T, index>;
    static constexpr bool eval = evaluated::value;
};


}

}

}
#endif
