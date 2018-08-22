#pragma once

#include "internal/platform.h"

#include "utility.h"

namespace estd {

#ifdef FEATURE_CPP_VARIADIC
template<class... TArgs>
class tuple;

template<std::size_t I, class T>
class tuple_element;


template< std::size_t I, class T >
class tuple_element< I, const T > {
  typedef typename
      std::add_const<typename std::tuple_element<I, T>::type>::type type;
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
};

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
}


template <class...Types>
tuple<Types...> make_tuple( Types&&... args )
{
    return tuple<Types...>(std::forward<Types>(args)...);
}


#endif

}
