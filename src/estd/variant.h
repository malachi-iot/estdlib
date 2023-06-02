#pragma once

#include "internal/platform.h"
#include "internal/functional.h"
#include "internal/variant.h"

#if __cpp_variadic_templates
namespace estd {

template <class ...Types>
using variant = internal::variant<Types...>;

template <unsigned index, class ...TArgs>
constexpr add_pointer_t<internal::type_at_index<index, TArgs...>> get_if(variant<TArgs...>* vs)
{
    return internal::holds_index<index>(vs) ? vs->template get<index>() : nullptr;
}


template <unsigned index, class ...TArgs>
constexpr add_pointer_t<const internal::type_at_index<index, TArgs...>> get_if(const variant<TArgs...>* vs)
{
    return internal::holds_index<index>(vs) ? vs->template get<index>() : nullptr;
}


template <class T, class ...Types>
constexpr add_pointer_t<T> get_if(variant<Types...>* vs) noexcept
{
    return internal::holds_type<T>(vs) ? vs->template get<T>() : nullptr;
}

template <class T, class ...Types>
constexpr add_pointer_t<const T> get_if(const variant<Types...>* vs) noexcept
{
    return internal::holds_type<T>(vs) ? vs->template get<T>() : nullptr;
}


template <int index, class ...TArgs>
internal::type_at_index<index, TArgs...>& get(variant<TArgs...>& v)
{
    internal::assert_index_matches<index>(v);

    return * v.template get<index>();
}


template <int index, class ...TArgs>
const internal::type_at_index<index, TArgs...>& get(const variant<TArgs...>& v)
{
    internal::assert_index_matches<index>(v);

    return * v.template get<index>();
}

template <int index, class ...TArgs>
internal::type_at_index<index, TArgs...>&& get(variant<TArgs...>&& v)
{
    internal::assert_index_matches<index>(v);

    return * v.template get<index>();
}


template <class T, class ...TArgs>
constexpr T& get(variant<TArgs...>& v)
{
    return get<internal::select_type<T, TArgs...>::index>(v);
}

template <class T, class ...TArgs>
constexpr const T& get(const variant<TArgs...>& v)
{
    return get<internal::select_type<T, TArgs...>::index>(v);
}

template <class T, class ...TArgs>
const T&& get(variant<TArgs...>&& v)
{
    return get<internal::select_type<T, TArgs...>::index>(v);
}


template< class T, class... Types >
constexpr bool holds_alternative(const variant<Types...>& v) noexcept
{
    return v.index() == internal::select_type<T, Types...>::selected_indices::first();
}


template <class... Types>
struct variant_size<variant<Types...> > :
    internal::variadic_size<Types...> {};

template <unsigned I, class... Types>
struct variant_alternative<I, internal::variant<Types...> > :
    type_identity<internal::type_at_index<I, Types...>> { };

}
#endif
