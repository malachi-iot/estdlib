#pragma once

#include "internal/platform.h"

#include "utility.h"
#include "type_traits.h"

namespace estd {

#ifdef FEATURE_CPP_VARIADIC
template<class... TArgs>
class tuple;

template<std::size_t I, class T>
class tuple_element;


template< std::size_t I, class T >
class tuple_element< I, const T > {
  typedef typename
      std::add_const<typename estd::tuple_element<I, T>::type>::type type;
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

    static CONSTEXPR int index = sizeof...(TArgs);

    typedef T element_type;

    const T& first() const { return value; }
};

// lifted from https://gist.github.com/IvanVergiliev/9639530
namespace internal {

template<int index, typename First, typename... Rest>
struct GetImpl
{
    static auto value(const tuple<First, Rest...>* t) ->
        decltype(GetImpl<index - 1, Rest...>::value(t))
    {
        return GetImpl<index - 1, Rest...>::value(t);
    }
};

template<typename First, typename... Rest>
struct GetImpl<0, First, Rest...>
{
    static First value(const tuple<First, Rest...>* t)
    {
        return t->first();
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

template<int index, typename First, typename... Rest>
auto get(const tuple<First, Rest...>& t) ->
    decltype(internal::GetImpl<index, First, Rest...>::value(&t))
{ //typename Type<index, First, Rest...>::value {
    return internal::GetImpl<index, First, Rest...>::value(&t);
}


/*
template< class T >
class tuple_size;

template< class T >
class tuple_size<const T>
 : public integral_constant<std::size_t, tuple_size<T>::value> { };
*/
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


#endif

}
