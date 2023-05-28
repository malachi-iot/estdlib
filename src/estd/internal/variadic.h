#pragma once
// Non-standard type_traits-like tools for parameter pack

#include "platform.h"

#if __cpp_variadic_templates

#include "../tuple.h"
#include "raw/type_traits.h"
#include "raw/utility.h"

namespace estd { namespace internal {

// Very similar to std::variant_alternative
template <int index, class ...TArgs>
using type_at_index = typename tuple_element<index, tuple<TArgs...> >::type;


template <unsigned I, class T>
struct visitor_index  :
    in_place_index_t<I>,
    in_place_type_t<T>
{};

template <typename... Types>
struct variadic_visitor_helper2
{
    template <int I, class F,
            class enabled = enable_if_t<(I == sizeof...(Types))>,
            class... TArgs>
    static constexpr bool visit(F&&, TArgs&&...) { return {}; }

    template <int I = sizeof...(Types) - 1, class F,
            class enabled = enable_if_t<(I < sizeof...(Types))>,
            class... TArgs>
    static bool visit(F&& f, TArgs&&...args, bool = true)
    {
        f(visitor_index<I, type_at_index<I, Types...>>{}, std::forward<TArgs>(args)...);
        return visit<I + 1>(std::forward<F>(f), std::forward<TArgs>(args)...);
    }
};

template <unsigned I, class F>
constexpr bool variadic_visitor_helper(F&&) { return {}; }

template <unsigned I, class F, class T, typename... Ts>
void variadic_visitor_helper(F&& f)
{
    f(in_place_index_t<I>{}, in_place_type_t<T>{});
    variadic_visitor_helper<I + 1, F, Ts...>(std::forward(f));
}

template <class F, typename... Ts>
void variadic_visitor(F&& f)
{
    return variadic_visitor_helper<0, F, Ts...>(std::forward(f));
}

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
