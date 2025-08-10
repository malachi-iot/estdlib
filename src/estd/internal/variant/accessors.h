#pragma once

#include "core.h"

namespace estd {

#if __cpp_variadic_templates
namespace internal {

template <size_t index, class ...Types>
constexpr bool holds_index(const variant<Types...>* vs)
{
    return vs != nullptr && vs->index() == index;
}


template <class T, class ...Types>
constexpr bool holds_type(const variant<Types...>* vs)
{
    typedef select_type<T, Types...> selected;

    // NOTE: size() check is redundant, because compile time check for 'first()' below fails
    // if no items are present.  This does not clash with spec, which indicates
    // this is "ill-formed if T is not a unique element"
    return selected::size() != 0 &&
           (vs != nullptr && vs->index() == selected::first::index);
}




template <int index, class ...Types>
void assert_index_matches(const variant<Types...>& v)
{
#if __cpp_exceptions
    if(v.index() != index) throw bad_variant_access();
#else
    // NOTE: Not tested yet
    if(v.index() != index)
#if FEATURE_STD
        std::abort();
#else
        abort();
#endif
#endif
}

template <int index, class ...Types>
inline ESTD_CPP_CONSTEXPR(14) type_at_index<index, Types...>& get(variant_storage<Types...>& vs)
{
    return * vs.template get<index>();
}

template <int index, class ...Types>
constexpr const type_at_index<index, Types...>& get(const variant_storage<Types...>& vs)
{
    return * vs.template get<index>();
}

template <class T, class ...Types>
inline ESTD_CPP_CONSTEXPR(14) T& get(variant_storage<Types...>& vs)
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
inline ESTD_CPP_CONSTEXPR(14) type_at_index<index, Types...>* get_ll(variant<Types...>& vs) noexcept
{
    return vs.template get<index>();
}


template <int index, class ...Types>
constexpr const type_at_index<index, Types...>* get_ll(const variant<Types...>& vs) noexcept
{
    return vs.template get<index>();
}

}

template <int index, class ...Types>
inline ESTD_CPP_CONSTEXPR(14) internal::type_at_index<index, Types...>& get(variant<Types...>& v)
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
inline ESTD_CPP_CONSTEXPR(14) T& get(variant<Types...>& v)
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
