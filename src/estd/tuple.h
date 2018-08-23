#pragma once

#include "internal/platform.h"

#include "utility.h"
#include "type_traits.h"

namespace estd {

#ifdef FEATURE_CPP_VARIADIC
template<class... TArgs>
class tuple;

template<std::size_t I, class T>
struct tuple_element;

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
class tuple<T, TArgs...> : public tuple<TArgs...>
{
    T value;

    typedef tuple<TArgs...> base_type;

public:
    tuple(T&& value, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...),
        value(std::move(value))
    {}

    explicit tuple() {}

    static CONSTEXPR int index = sizeof...(TArgs);

    typedef T element_type;

    const T& first() const { return value; }

    T& first() { return value; }
};

// lifted from https://gist.github.com/IvanVergiliev/9639530
namespace internal {

template<int index, typename First, typename... Rest>
struct GetImpl
{
    typedef GetImpl<index - 1, Rest...> parent_type;
    typedef First first_type;

    static auto value(const tuple<First, Rest...>& t) ->
        decltype(GetImpl<index - 1, Rest...>::value(t))
    {
        return GetImpl<index - 1, Rest...>::value(t);
    }

    static typename parent_type::first_type& value(tuple<First, Rest...>& t)
    {
        return parent_type::value(t);
    }
};

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

template< class T >
struct tuple_size
        : estd::integral_constant<std::size_t, T::index + 1>
{
    //static constexpr unsigned value();// T::index;
};


template <class...Types>
tuple<Types...> make_tuple( Types&&... args )
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
                  Tuple
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




#endif

}
