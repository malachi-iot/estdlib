#pragma once

#include "internal/platform.h"
#include "internal/fwd/tuple.h"

#include "type_traits.h"

#ifdef FEATURE_CPP_VARIADIC
namespace estd {

template <std::size_t I, class T>
using tuple_element_t = typename tuple_element<I, T>::type;


// recursive case
template< std::size_t I, class Head, class... Tail >
struct tuple_element<I, estd::tuple<Head, Tail...>>
    : estd::tuple_element<I-1, estd::tuple<Tail...>> { };

// base case
template< class Head, class... Tail >
struct tuple_element<0, estd::tuple<Head, Tail...>> {
   typedef Head type;
};

template< std::size_t I, class T >
struct tuple_element< I, const T > {
  typedef typename
      estd::add_const<typename estd::tuple_element<I, T>::type>::type type;
};


template <>
class tuple<>
{
public:
    //static CONSTEXPR int index = 0;
};

template <class T, class ...TArgs>
class tuple<T&, TArgs...> : public tuple<TArgs...>
{
    T& value;

    typedef tuple<TArgs...> base_type;

public:
    constexpr tuple(T& value, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...),
        value(value)
    {}

    static constexpr int index = sizeof...(TArgs);

    const T& first() const { return value; }

    T& first() { return value; }
};

namespace internal {

#ifndef FEATURE_ESTD_SPARSE_TUPLE
#define FEATURE_ESTD_SPARSE_TUPLE 1
#endif

#if FEATURE_ESTD_IS_EMPTY && FEATURE_ESTD_SPARSE_TUPLE
template <class T, unsigned index, class enabled = void>
struct sparse_tuple;

template <class T, unsigned index>
struct sparse_tuple<T, index, typename enable_if<is_empty<T>::value>::type>
{
    static T first() { return T(); }
};


template <class T, unsigned index>
struct sparse_tuple<T, index, typename enable_if<!is_empty<T>::value>::type>
#else
template <class T, unsigned index>
struct sparse_tuple
#endif
{
    T value;

    const T& first() const { return value; }

    T& first() { return value; }

    constexpr sparse_tuple(T&& value) : value(std::move(value)) {}
    constexpr sparse_tuple() = default;
};

}


template <class T, class ...TArgs>
class tuple<T, TArgs...> :
    public tuple<TArgs...>,
    public internal::sparse_tuple<T, sizeof...(TArgs)>
{
    typedef tuple<TArgs...> base_type;
    typedef internal::sparse_tuple<T, sizeof...(TArgs)> storage_type;

public:
    constexpr tuple(T&& value, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...),
        storage_type(std::move(value))
    {}

    using storage_type::first;

    explicit tuple() {}

    static constexpr int index = sizeof...(TArgs);

    typedef T element_type;
};

// lifted and adapted from https://gist.github.com/IvanVergiliev/9639530
namespace internal {

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

    static const First& value(const tuple<First, Rest...>& t)
    {
        return t.first();
    }

    static First& value(tuple<First, Rest...>& t)
    {
        return t.first();
    }

    static First&& value(tuple<First, Rest...>&& t)
    {
        return t.first();
    }
};


}

/*
template< std::size_t I, class... Types >
typename tuple_element<I, tuple<Types...> >::type&
    get( const tuple<Types...>& t ) noexcept
{
    if(t.index == I)
    {
        tuple_element<I, tuple<Types...> > e;
        return e;
    }
    else
        return get<I - 1, tuple<Types...>::base_type>(t);
} */


template<int index, typename... Types>
const tuple_element_t<index, tuple<Types...> >& get(const tuple<Types...>& t)
{
    return internal::GetImpl<index, Types...>::value(t);
}

template<int index, typename... Types>
tuple_element_t<index, tuple<Types...> >& get(tuple<Types...>& t)
{
    return internal::GetImpl<index, Types...>::value(t);
}

template<int index, typename... Types>
tuple_element_t<index, tuple<Types...> >&& get(tuple<Types...>&& t)
{
    return internal::GetImpl<index, Types...>::value(std::move(t));
}

template<int index, typename... Types>
const tuple_element_t<index, tuple<Types...> >&& get(const tuple<Types...>&& t)
{
    return internal::GetImpl<index, Types...>::value(t);
}

#if OLD_VERSION
// I think this was used with hopes of c++03 friendliness.  Since none of the rest of
// tuple support is c++03 friendly, phasing this out should be harmless
template < class T >
struct tuple_size
        : estd::integral_constant<std::size_t, T::index + 1>
{
    //static constexpr unsigned value();// T::index;
};
#else
template <class ...TArgs>
struct tuple_size<estd::tuple<TArgs...> >
    : estd::integral_constant<std::size_t, sizeof...(TArgs)>
{
};

#endif




template <class...Types>
CONSTEXPR tuple<Types...> make_tuple( Types&&... args )
{
    return tuple<Types...>(std::forward<Types>(args)...);
}

namespace internal {

// FIX: important deviation from spec in that we aren't returning
// a value.  That is a little tricky
template <class F2, class Tuple, size_t... Is>
inline auto apply_impl(F2&& f, Tuple&& t, index_sequence<Is...>) ->
    decltype (f(get<Is>(t)...))
{
    return f(get<Is>(t)...);
}

}

template <class F2, class Tuple, class TSeq = make_index_sequence<
              tuple_size<
                  remove_reference_t<Tuple>
              >::value> >
inline auto apply(F2&& f, Tuple&& t) ->
    decltype (internal::apply_impl(std::move(f),
                                   std::forward<Tuple>(t),
                                   TSeq {}))
{
    return internal::apply_impl(std::move(f),
               std::forward<Tuple>(t),
               TSeq {});
}


}

#else
#include "c++03/tuple.h"
#endif
