#pragma once

#include "internal/platform.h"
#include "internal/functional.h"
#include "internal/variant.h"

#if __cpp_variadic_templates
namespace estd {

template <class ...Types>
using variant = internal::variant<Types...>;

template <unsigned index, class ...Types>
constexpr add_pointer_t<internal::type_at_index<index, Types...>> get_if(variant<Types...>* vs)
{
    return internal::holds_index<index>(vs) ? internal::get_ll<index>(*vs) : nullptr;
}


template <unsigned index, class ...Types>
constexpr add_pointer_t<const internal::type_at_index<index, Types...>> get_if(const variant<Types...>* vs)
{
    return internal::holds_index<index>(vs) ? internal::get_ll<index>(*vs) : nullptr;
}


template <class T, class ...Types>
constexpr add_pointer_t<T> get_if(variant<Types...>* vs) noexcept
{
    return get_if<internal::select_type<T, Types...>::first::index>(vs);
}

template <class T, class ...Types>
constexpr add_pointer_t<const T> get_if(const variant<Types...>* vs) noexcept
{
    return get_if<internal::select_type<T, Types...>::first::index>(vs);
}


template <int index, class ...Types>
internal::type_at_index<index, Types...>& get(variant<Types...>& v)
{
    internal::assert_index_matches<index>(v);

    return * internal::get_ll<index>(v);
}


template <int index, class ...Types>
const internal::type_at_index<index, Types...>& get(const variant<Types...>& v)
{
    internal::assert_index_matches<index>(v);

    return * internal::get_ll<index>(v);
}

template <int index, class ...Types>
internal::type_at_index<index, Types...>&& get(variant<Types...>&& v)
{
    internal::assert_index_matches<index>(v);

    return * internal::get_ll<index>(v);
}


template <class T, class ...Types>
constexpr T& get(variant<Types...>& v)
{
    return get<internal::select_type<T, Types...>::first::index>(v);
}

template <class T, class ...Types>
constexpr const T& get(const variant<Types...>& v)
{
    return get<internal::select_type<T, Types...>::first::index>(v);
}

template <class T, class ...Types>
T&& get(variant<Types...>&& v)
{
    return get<internal::select_type<T, Types...>::first::index>(v);
}


template< class T, class... Types >
constexpr bool holds_alternative(const variant<Types...>& v) noexcept
{
    return v.index() == internal::select_type<T, Types...>::first::index;
}


template <class... Types>
struct variant_size<variant<Types...> > : variadic::size<Types...> {};

template <size_t I, class... Types>
struct variant_alternative<I, variant<Types...> > :
    type_identity<internal::type_at_index<I, Types...>> { };

}
#endif
