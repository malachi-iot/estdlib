#pragma once

#include "platform.h"
#include "is_empty.h"
#include "type_traits.h"
#include "fwd/tuple.h"

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

    typedef T return_type;
    typedef T const_return_type;
};


template <class T, std::size_t index>
struct sparse_tuple<T, index, typename enable_if<!is_empty<T>::value>::type>
#else
template <class T, std::size_t index>
struct sparse_tuple
#endif
{
    T value;

    typedef T& return_type;
    typedef const T& const_return_type;

    const T& first() const { return value; }

    T& first() { return value; }

    constexpr sparse_tuple(T&& value) : value(std::move(value)) {}
    constexpr sparse_tuple() = default;
};



// 'GetImpl' lifted and adapted from https://gist.github.com/IvanVergiliev/9639530

template<int index, typename First, typename... Rest>
struct GetImpl : GetImpl<index - 1, Rest...>
{
    typedef GetImpl<index - 1, Rest...> base_type;
    typedef typename base_type::first_type first_type;

    static const first_type& value(const tuple<First, Rest...>& t)
    {
        return base_type::value(t);
    }

    static first_type& value(tuple<First, Rest...>& t)
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
    using return_type = typename tuple<First, Rest...>::return_type;
    using const_return_type = typename tuple<First, Rest...>::const_return_type;

    static const_return_type value(const tuple<First, Rest...>& t)
    {
        return t.first();
    }

    static return_type value(tuple<First, Rest...>& t)
    {
        return t.first();
    }

    /*
    static First& value(tuple<First, Rest...>& t)
    {
        return t.first();
    } */

    static First&& value(tuple<First, Rest...>&& t)
    {
        return t.first();
    }
};

}}
