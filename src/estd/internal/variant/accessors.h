#pragma once

#include "core.h"

namespace estd {

#if __cpp_variadic_templates
namespace internal {

template <int index, class ...Types>
type_at_index<index, Types...>& get(variant_storage<Types...>& vs)
{
    return * vs.template get<index>();
}

template <int index, class ...Types>
const type_at_index<index, Types...>& get(const variant_storage<Types...>& vs)
{
    return * vs.template get<index>();
}

template <class T, class ...Types>
T& get(variant_storage<Types...>& vs)
{
    return * vs.template get<T>();
}

template <class T, class ...Types>
constexpr const T& get(const variant_storage<Types...>& vs)
{
    return * vs.template get<T>();
}


// Bypasses runtime index check
template <int index, class ...Types>
type_at_index<index, Types...>* get_ll(variant<Types...>& vs) noexcept
{
    return vs.template get<index>();
}


template <int index, class ...Types>
constexpr const type_at_index<index, Types...>* get_ll(const variant<Types...>& vs) noexcept
{
    return vs.template get<index>();
}


}
#else
namespace internal {


template <size_t I, class T1, class T2, class T3>
typename get_type_at_index<I, T1, T2, T3>::type& get(variant_storage<T1, T2, T3>& vs)
{
    return * vs.template get<I>();
}

template <size_t I, class T1, class T2, class T3>
const typename get_type_at_index<I, T1, T2, T3>::type& get(const variant_storage<T1, T2, T3>& vs)
{
    return * vs.template get<I>();
}

}
#endif

}
