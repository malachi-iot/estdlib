#pragma once

#include "internal/tuple.h"

#include "type_traits.h"

#if __cpp_alias_templates && __cpp_variadic_templates
namespace estd {

// TODO: Feed tuple_element from new variadic support code

// recursive case
template< std::size_t I, class Head, class... Tail >
struct tuple_element<I, estd::tuple<Head, Tail...>>
    : estd::tuple_element<I-1, estd::tuple<Tail...>> { };

// base case
template< class Head, class... Tail >
struct tuple_element<0, estd::tuple<Head, Tail...>>
{
    using valref_type = typename estd::tuple<Head>::valref_type;
    using const_valref_type = typename estd::tuple<Head>::const_valref_type;

    typedef Head type;
};

template< std::size_t I, class T >
struct tuple_element< I, const T >
{
    typedef typename estd::add_const<typename estd::tuple_element<I, T>::type>::type type;
};




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




}

#else
#include "c++03/tuple.h"
#endif
