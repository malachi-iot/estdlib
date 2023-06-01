#pragma once
// Non-standard type_traits-like tools for parameter pack

#include "platform.h"
#include "variadic/integer_sequence.h"

#if __cpp_variadic_templates

#include "../tuple.h"
#include "raw/type_traits.h"
#include "raw/utility.h"
#include "raw/variant.h"

namespace estd { namespace internal {

// Indicates the function/constructor expects a functor for iterating
// over all the variadic possibilities
struct in_place_visit_t : in_place_tag {};

struct eval_result_tag {};

template <size_t pos, class ...Types>
struct get_type_finder;

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
};


// Very similar to std::variant_alternative
template <size_t index, class ...Types>
using type_at_index = typename get_type_finder<index, Types...>::type;

template <int ...Is>
struct indices_reverser;



template <int ...Is>
using indices = integer_sequence<int, Is...>;

template <>
struct indices_reverser<>
{
    using reversed = indices<>;
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



template <unsigned I, class T>
struct visitor_index  :
    in_place_index_t<I>,
    in_place_type_t<T>
{};

template <unsigned I, class T>
struct visitor_instance : visitor_index<I, T>
{
    T& value;

    visitor_instance(const visitor_instance&) = default;
    constexpr explicit visitor_instance(T& value) : value{value} {}
};


#if __cpp_concepts
#include <concepts>

namespace concepts {

template <class T, class TProvider>
concept GetterFunctor = requires(T f, TProvider& provider)
{
    f(visitor_index<0, int>{}, provider);
};

template <class T, class ...TArgs>
concept ClassVisitorFunctor = requires(T f, TArgs&&...args)
{
    { f(visitor_index<0, int>{}, std::forward<TArgs>(args)...) } -> std::same_as<bool>;
};


template <class T, class ...TArgs>
concept InstanceVisitorFunctor = requires(T f, TArgs&&...args, int v)
{
    { f(visitor_instance<0, int>(v), std::forward<TArgs>(args)...) } -> std::same_as<bool>;
};

}
#endif

template <class TEval, class ...Types>
struct visitor_helper_struct;


// EXPERIMENTAL, perhaps we prefer this specialization over functor?
template <class TProvider>
struct visitor_instance_factory
{
    template <unsigned I, class T>
    visitor_instance<I, T> create(visitor_index<I, T>) { return {}; }
};


template <typename... Types>
struct variadic_visitor_helper2
{
    template <int I,
            class enabled = enable_if_t<(I == sizeof...(Types))>,
            class... TArgs,
#if __cpp_concepts
            concepts::ClassVisitorFunctor<TArgs...> F>
#else
            class F>
#endif
    static constexpr short visit(F&&, TArgs&&...) { return -1; }

    template <int I = 0, class F,
            class enabled = enable_if_t<(I < sizeof...(Types))>,
            class... TArgs>
    static int visit(F&& f, TArgs&&...args)
    {
        if(f(visitor_index<I, type_at_index<I, Types...>>{}, std::forward<TArgs>(args)...))
            return I;

        return visit<I + 1>(std::forward<F>(f), std::forward<TArgs>(args)...);
    }


#if __cpp_concepts
    template <int I, class... TArgs,
        concepts::InstanceVisitorFunctor<TArgs...> F, class T,
        concepts::GetterFunctor<T&> Fg,
#else
    template <int I, class F, class Fg, class T, class... TArgs,
#endif
            class enabled = enable_if_t<(I == sizeof...(Types))> >
    static constexpr short visit_instance(F&&, Fg&&, T&, TArgs&&...) { return -1; }

    template <int I = 0, class F, class Fg, class T,
            class enabled = enable_if_t<(I < sizeof...(Types))>,
            class... TArgs>
    static int visit_instance(F&& f, Fg&& getter, T& t, TArgs&&...args)
    {
        typedef type_at_index<I, Types...> value_type;
        visitor_instance<I, value_type> vi{getter(visitor_index<I, value_type>{}, t)};

        if(f(vi, std::forward<TArgs>(args)...))
            return I;

        return visit_instance<I + 1>(
            std::forward<F>(f),
            std::forward<Fg>(getter),
            t,
            std::forward<TArgs>(args)...);
    }

    template <class TEval>
    using visit_struct = visitor_helper_struct<TEval, Types...>;
};

template <class ...Types>
struct variadic_first;

template <class T, class ...Types>
struct variadic_first<T, Types...> : type_identity<T> {};

template <unsigned size, class TEval, class ...Types>
struct visitor_helper_struct2;

template <unsigned size, class TEval>
struct visitor_helper_struct2<size, TEval>
{
    static constexpr ptrdiff_t selected = -1;

    // DEBT: Monostate may collide with seeked-for types
    typedef monostate selected_type;

    using selected_indices = index_sequence<>;
    using selected_types = type_sequence<>;
    using projected_types = type_sequence<>;
};


template <unsigned size, class TEval, class T, class ...Types>
struct visitor_helper_struct2<size, TEval, T, Types...>
{
    typedef visitor_helper_struct2<size, TEval, Types...> upward;

    static constexpr size_t index = ((size - 1) - sizeof...(Types));

    using evaluated = typename TEval::template evaluator<T, index>;
    static constexpr bool eval = evaluated::value;
    static constexpr ptrdiff_t selected = eval ? index : upward::selected;

    using projected_type = conditional_t<
            is_base_of<eval_result_tag, evaluated>::value,
            typename evaluated::type, T>;
    using selected_type = conditional_t<eval, projected_type, typename upward::selected_type>;

    using selected_indices = conditional_t<eval,
        typename upward::selected_indices::template prepend<index>,
        typename upward::selected_indices>;

    using selected_types = conditional_t<eval,
        typename upward::selected_types::template prepend<T>,
        typename upward::selected_types>;

    using projected_types = conditional_t<eval,
        typename upward::projected_types::template prepend<projected_type>,
        typename upward::projected_types>;
};

template <class TEval, class ...Types>
struct visitor_helper_struct
{
    typedef visitor_helper_struct2<sizeof...(Types), TEval, Types...> vh_type;

    static constexpr ptrdiff_t selected = vh_type::selected;
    static constexpr ptrdiff_t index = selected;

    using selected_type = typename vh_type::selected_type;
    using selected_indices = typename vh_type::selected_indices;

    static constexpr unsigned found = selected_indices::size();
    static constexpr bool multiple = found > 1;

    // DEBT: fix signed/unsigned here
    using visitor_index = internal::visitor_index<(unsigned)index, selected_type>;
};

template <bool v, class T>
struct projected_result :
    type_identity<T>
{
    constexpr static bool value = v;
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
template <class ...TArgs>
struct are_trivial;

template <>
struct are_trivial<>
{
    static constexpr bool value = true;
};

template <class T, class ...TArgs>
struct are_trivial<T, TArgs...>
{
    static constexpr bool value = estd::is_trivial<T>::value &
        are_trivial<TArgs...>::value;
};


template <class T, int I, class ...TArgs>
struct index_of_type_helper;

template <class T, int I>
struct index_of_type_helper<T, I>
{
    static constexpr int index = -1;
};

template <class T, int I, class T2, class ...TArgs>
struct index_of_type_helper<T, I, T2, TArgs...>
{
private:
    typedef index_of_type_helper<T, I + 1, TArgs...> up_one;

    static constexpr bool match = is_same<T, T2>::value;

public:
    static constexpr int index = match ? I : up_one::index;
    static constexpr bool multiple = match & (up_one::index != -1);
};

template <class T, class ...TArgs>
using index_of_type = visitor_helper_struct<is_same_selector<T>, TArgs...>;
//using index_of_type = index_of_type_helper<T, 0, TArgs...>;

// Evaluate true or false whether a variadic contains a particular type
template <class T, class ...Types>
using contains_type = conditional_t<
        index_of_type<T, Types...>::index == -1,
        false_type,
        true_type>;

// Only enables if type T is contained in Types
template <class T, class ...Types>
using ensure_type = enable_if<contains_type<T, Types...>::value, T>;

template <class T, class ...Types>
using ensure_type_t = typename ensure_type<T, Types...>::type;

// Wrapper to help return specifically as an integral constant
template <class ...Types>
using variadic_size = integral_constant<std::size_t, sizeof...(Types)>;

}}
#endif
