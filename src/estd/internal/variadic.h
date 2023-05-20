#pragma once
// Non-standard type_traits-like tools for parameter pack

#include "platform.h"

#if __cpp_variadic_templates

#include "../tuple.h"
#include "raw/type_traits.h"

namespace estd { namespace internal {


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


// Very similar to std::variant_alternative
template <int index, class ...TArgs>
using type_at_index = typename tuple_element<index, tuple<TArgs...> >::type;

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

}}
#endif
