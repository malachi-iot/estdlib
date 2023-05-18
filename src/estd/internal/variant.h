#pragma once

#include "platform.h"
#include "fwd/functional.h"
#include "utility.h"
#include "raw/variant.h"
#include "../tuple.h"

namespace estd {

#if __cpp_variadic_templates
namespace internal {

// largest_type lifted from
// https://stackoverflow.com/questions/16803814/how-do-i-return-the-largest-type-in-a-list-of-types
// DEBT: Put this helper elsewhere
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
    using type = typename largest_type<typename std::conditional<
        (sizeof(U) <= sizeof(T)), T, U>::type, Ts...
    >::type;
};

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


template <int index, class ...TArgs>
using type_at_index = typename tuple_element<index, tuple<TArgs...> >::type;


template <bool trivial, class ...T>
struct variant_storage;

template <int index, bool trivial, class ...TArgs>
type_at_index<index, TArgs...>& get(variant_storage<trivial, TArgs...>& vs)
{
    typedef type_at_index<index, TArgs...>* pointer;
    return * (pointer)vs.raw;
}

template <int index, bool trivial, class ...TArgs>
const type_at_index<index, TArgs...>& get(const variant_storage<trivial, TArgs...>& vs)
{
    typedef const type_at_index<index, TArgs...>* const_pointer;
    return * (const_pointer)vs.raw;
}


// DEBT: Currently variant_storage is clunky and fine-tuned for consumption by
// 'expected'


template <class T1, class T2>
struct variant_storage<true, T1, T2>
{
    typedef tuple<T1, T2> tuple_type;
    static constexpr bool is_trivial = true;

    union
    {
        T1 t1;
        T2 t2;
        byte raw[0];
    };

    variant_storage() = default;

    /*
    template <unsigned index, class ...TArgs>
    variant_storage(estd::in_place_index_t<index>, TArgs&&...args)
    {
        typedef type_at_index<index, T1, T2> type;
        new (& get<index>(*this)) type(std::forward<TArgs>(args)...);
    } */

    template <class ...TArgs>
    constexpr variant_storage(estd::in_place_index_t<0>, TArgs&&...args) :
        t1(std::forward<TArgs>(args)...)
    {
    }

    template <class ...TArgs>
    constexpr variant_storage(estd::in_place_index_t<1>, TArgs&&...args) :
        t2(std::forward<TArgs>(args)...)
    {
    }
};

template <class ...T>
struct variant_storage<false, T...>
{
    typedef tuple<T...> tuple_type;
    static constexpr bool is_trivial = false;

    estd::byte raw[sizeof(typename largest_type<T...>::type)];

    variant_storage() = default;

    template <unsigned index, class ...TArgs>
    variant_storage(estd::in_place_index_t<index>, TArgs&&...args)
    {
        typedef type_at_index<index, T...> type;
        new (& get<index>(*this)) type(std::forward<TArgs>(args)...);
    }
};

template <class ...T>
using variant_storage2 = variant_storage<are_trivial<T...>::value, T...>;


}
#endif

}
