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

template <int index, bool trivial, class ...TArgs>
struct get_variant_storage_helper;



template <bool trivial, class ...T>
struct variant_storage;

template <int index, bool trivial, class ...TArgs>
type_at_index<index, TArgs...>& get(variant_storage<trivial, TArgs...>& vs)
{
    return get_variant_storage_helper<index, trivial, TArgs...>::get(vs);
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
    };

    T1* get0() { return &t1; }
    T2* get1() { return &t2; }

    variant_storage() = default;

    template <estd::size_t index, class ...TArgs>
    variant_storage(estd::in_place_index_t<index>, TArgs&&...args)
    {
        typedef type_at_index<index, T1, T2> type;
        new (& get<index>(*this)) type(std::forward<TArgs>(args)...);
    }
};

template <class ...T>
struct variant_storage<false, T...>
{
    typedef tuple<T...> tuple_type;
    typedef tuple_element<0, tuple_type> t1_type;
    typedef tuple_element<1, tuple_type> t2_type;
    typedef tuple_element<2, tuple_type> t3_type;
    static constexpr bool is_trivial = false;

    estd::byte raw[sizeof(typename largest_type<T...>::type)];

    t1_type* get0() { return (t1_type*)raw; }
    t2_type* get1() { return (t2_type*)raw; }
    t3_type* get2() { return (t3_type*)raw; }

    variant_storage() = default;

    template <estd::size_t index, class ...TArgs>
    variant_storage(estd::in_place_index_t<index>, TArgs&&...args)
    {
        typedef type_at_index<index, T...> type;
        new (& get<index>(*this)) type(std::forward<TArgs>(args)...);
    }
};

template <class ...T>
using variant_storage2 = variant_storage<are_trivial<T...>::value, T...>;



template <bool trivial, class ...TArgs>
struct get_variant_storage_helper<0, trivial, TArgs...>
{
    static type_at_index<0, TArgs...>& get(variant_storage<trivial, TArgs...>& vs)
    {
        return * vs.get0();
    }
};


template <bool trivial, class ...TArgs>
struct get_variant_storage_helper<1, trivial, TArgs...>
{
    static type_at_index<1, TArgs...>& get(variant_storage<trivial, TArgs...>& vs)
    {
        return * vs.get1();
    }
};




}
#endif

}
