#pragma once
// Non-standard type_traits-like tools for parameter pack

#include "platform.h"

#if __cpp_variadic_templates

#include "../tuple.h"
#include "raw/type_traits.h"
#include "raw/utility.h"
#include "raw/variant.h"

namespace estd { namespace internal {

// Indicates the function/constructor expects a functor for iterating
// over all the variadic possibilities
struct in_place_visit_t : in_place_tag {};


// Very similar to std::variant_alternative
template <int index, class ...TArgs>
using type_at_index = typename tuple_element<index, tuple<TArgs...> >::type;


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
};

template <class TEval, class ...Types>
struct visitor_helper_struct;

template <class TEval>
struct visitor_helper_struct<TEval>
{
    static constexpr int selected = -1;
};


template <class TEval, class T, class ...Types>
struct visitor_helper_struct<TEval, T, Types...>
{
    typedef visitor_helper_struct<TEval, Types...> upward;
    static constexpr int index = sizeof...(Types) - 1;
    static constexpr bool eval = TEval::template evaluator<T>::value;
    static constexpr int selected = eval ? index : upward::selected;
};


// EXPERIMENTAL
template <class T>
struct converting_selector
{
    template <class T_j>
    using evaluator = is_convertible<T_j, T>;
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
using index_of_type = index_of_type_helper<T, 0, TArgs...>;

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
