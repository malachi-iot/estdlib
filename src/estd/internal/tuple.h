#pragma once

#include "platform.h"
#include "is_empty.h"
#include "type_traits.h"
#include "fwd/tuple.h"

#if defined(FEATURE_STD_UTILITY)
// rpi-pico needs an explicit include of this here for std::move
#include <utility>
#endif

namespace estd { namespace internal {

#ifndef FEATURE_ESTD_SPARSE_TUPLE
#define FEATURE_ESTD_SPARSE_TUPLE 1
#endif

// DEBT: 'sparse_tuple' can probably work well in c++03 tuple flavor too

// Needs 'index' to disambiguate from multiple base classes
#if FEATURE_ESTD_IS_EMPTY && FEATURE_ESTD_SPARSE_TUPLE
template <class T, std::size_t index, class enabled = void>
struct sparse_tuple;

template <class T, std::size_t index>
struct sparse_tuple<T, index, typename enable_if<is_empty<T>::value>::type>
{
    static T first() { return T(); }

    typedef T valref_type;
    typedef T const_valref_type;
};


template <class T, std::size_t index>
struct sparse_tuple<T, index, typename enable_if<!is_empty<T>::value>::type>
#else
template <class T, std::size_t index>
struct sparse_tuple
#endif
{
    T value;

    typedef T& valref_type;
    typedef const T& const_valref_type;

    const T& first() const { return value; }

    T& first() { return value; }

#if __cpp_constexpr
    constexpr sparse_tuple(T&& value) : value(std::move(value)) {}
    constexpr sparse_tuple() = default;
#else
    sparse_tuple(const T& value) : value(value) {}
    sparse_tuple() {};
#endif
};


#if __cpp_variadic_templates
// 'GetImpl' lifted and adapted from https://gist.github.com/IvanVergiliev/9639530
// TODO: use new variadic support area instead of this

template<int index, typename First, typename... Rest>
struct GetImpl : GetImpl<index - 1, Rest...>
{
    typedef GetImpl<index - 1, Rest...> base_type;

    static typename base_type::tuple_type::const_valref_type value(const tuple<First, Rest...>& t)
    {
        return base_type::value(t);
    }

    static typename base_type::tuple_type::valref_type value(tuple<First, Rest...>& t)
    {
        return base_type::value(t);
    }
};

// cascades down eventually to this particular specialization
// to retrieve the 'first' item
template<typename First, typename... Rest>
struct GetImpl<0, First, Rest...>
{
    typedef First first_type;
    using tuple_type = tuple<First, Rest...>;
    using valref_type = typename tuple<First>::valref_type;
    using const_valref_type = typename tuple<First>::const_valref_type;

    static const_valref_type value(const tuple<First, Rest...>& t)
    {
        return t.first();
    }

    static valref_type value(tuple<First, Rest...>& t)
    {
        return t.first();
    }

    static First&& value(tuple<First, Rest...>&& t)
    {
        return t.first();
    }
};
#endif

}}
