#pragma once

#include "platform.h"
#include "fwd/functional.h"
#include "utility.h"
#include "raw/variant.h"
#include "../tuple.h"

namespace estd {

#if __cpp_variadic_templates
namespace internal {


// Very similar to std::variant_alternative
template <int index, class ...TArgs>
using type_at_index = typename tuple_element<index, tuple<TArgs...> >::type;


template <bool trivial, class ...T>
struct variant_storage;

template <int index, bool trivial, class ...TArgs>
type_at_index<index, TArgs...>& get(variant_storage<trivial, TArgs...>& vs)
{
    typedef type_at_index<index, TArgs...>* pointer;
    return * (pointer)vs.storage.raw;
}

template <int index, bool trivial, class ...TArgs>
const type_at_index<index, TArgs...>& get(const variant_storage<trivial, TArgs...>& vs)
{
    typedef const type_at_index<index, TArgs...>* const_pointer;
    return * (const_pointer)vs.storage.raw;
}


template <class T, unsigned I, class ...TArgs>
struct index_of_type_helper;

template <class T>
struct index_of_type_helper<T, 0>
{
    static constexpr bool match = false;
    static constexpr int index = -1;
};

template <class T, unsigned I, class T2, class ...TArgs>
struct index_of_type_helper<T, I, T2, TArgs...>
{
    typedef index_of_type_helper<T, I - 1, TArgs...> up_one;

    static constexpr bool match = is_same<T, T2>::value;
    static constexpr int index = match ? I : up_one::index;
};

template <class T, class ...TArgs>
using index_of_type = index_of_type_helper<T, sizeof...(TArgs), TArgs...>;


// DEBT: Currently variant_storage is clunky and fine-tuned for consumption by
// 'expected'

// Very similar to c++20 flavor, but importantly returns monostate instead
template <class T2, class ...TArgs>
inline static monostate construct_at(void* placement, TArgs&&...args)
{
    new (placement) T2(std::forward<TArgs>(args)...);
    return {};
}


template <bool trivial, class ...TArgs>
union variant_union;

template <class ...T>
union variant_union<false, T...>
{
    estd::byte raw[sizeof(typename largest_type<T...>::type)];
};

// Not 100% needed, but I like that I can see values more
// easily in the debugger this way
template <class T1, class T2>
union variant_union<true, T1, T2>
{
    T1 t1;
    T2 t2;
    byte raw[0];
};

template <class T1, class T2, class T3>
union variant_union<true, T1, T2, T3>
{
    T1 t1;
    T2 t2;
    T3 t3;
    byte raw[0];
};


template <bool trivial, class ...T>
struct variant_storage
{
    typedef tuple<T...> tuple_type;
    static constexpr bool is_trivial = trivial;

    union
    {
        variant_union<trivial, T...> storage;
        monostate dummy;
    };

    variant_storage() = default;

    template <unsigned index, class ...TArgs>
    constexpr variant_storage(estd::in_place_index_t<index>, TArgs&&...args) :
        dummy{
            construct_at<type_at_index<index, T...>>
                (storage.raw, std::forward<TArgs>(args)...)}
    {
    }
};

template <class ...T>
using variant_storage2 = variant_storage<are_trivial<T...>::value, T...>;


}
#endif

}
