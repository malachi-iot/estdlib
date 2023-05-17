#pragma once

#include "platform.h"
#include "fwd/functional.h"
#include "raw/variant.h"
#include "tuple.h"

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
            (sizeof(U) <= sizeof(T)), T, U
    >::type, Ts...
    >::type;
};

template <bool trivial, class ...T>
struct variant_storage;

// DEBT: Currently variant_storage is clunky and fine-tuned for consumption by
// 'expected'

template <class T>
struct variant_storage<true, void, T>
{
    typedef tuple<void, T> tuple_type;

    T t2;

    T* get1() { return &t2; }
};


template <class T1, class T2>
struct variant_storage<true, T1, T2>
{
    typedef tuple<T1, T2> tuple_type;

    union
    {
        T1 t1;
        T2 t2;
    };

    T1* get0() { return &t1; }
    T2* get1() { return &t2; }
};

template <class ...T>
struct variant_storage<false, T...>
{
    typedef tuple<T...> tuple_type;
    typedef tuple_element<0, tuple_type> t1_type;
    typedef tuple_element<1, tuple_type> t2_type;

    estd::byte raw[sizeof(largest_type<T...>::value)];

    t1_type* get0() { return (t1_type*)raw; }
    t2_type* get1() { return (t2_type*)raw; }
};


template <int index, class ...TArgs>
using type_at_index = typename tuple_element<index, tuple<TArgs...> >::type;


template <int index, bool trivial, class ...TArgs>
struct get_variant_storage_helper;

template <bool trivial, class ...TArgs>
struct get_variant_storage_helper<0, trivial, TArgs...>
{
    static type_at_index<0, TArgs...>& get(const variant_storage<trivial>& vs)
    {
        return vs.get0();
    }
};


// NOTE: This will need a struct helper since partial specialization is not allowed
template <int index, bool trivial, class ...TArgs>
type_at_index<index, TArgs...> get(const variant_storage<trivial, TArgs...>&);


}
#endif

}