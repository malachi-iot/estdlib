#pragma once

#include "internal/tuple.h"

// EXPERIMENTAL
#include "internal/variadic/visitor.h"

#include "type_traits.h"

#if __cpp_alias_templates && __cpp_variadic_templates
namespace estd {

template <std::size_t I, class T>
using tuple_element_t = typename tuple_element<I, T>::type;


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
    using types = variadic::types<T&, TArgs...>;

public:
    typedef T& valref_type;
    typedef const T& const_valref_type;

    constexpr explicit tuple(T& value, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...),
        value(value)
    {}

    static constexpr int index = sizeof...(TArgs);

    const T& first() const { return value; }

    T& first() { return value; }
};




template <class T, class ...TArgs>
class tuple<T, TArgs...> :
    public tuple<TArgs...>,
    public internal::sparse_tuple<T, sizeof...(TArgs)>
{
    typedef tuple<TArgs...> base_type;
    typedef internal::sparse_tuple<T, sizeof...(TArgs)> storage_type;
    using types = variadic::types<T, TArgs...>;

public:
    template <class UType,
        enable_if_t<is_constructible<T, UType>::value, bool> = true>
    constexpr tuple(UType&& value, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...),
        storage_type(std::forward<UType>(value))
    {}

    constexpr tuple(T&& value, TArgs&&...args) :
        base_type(std::forward<TArgs>(args)...),
        storage_type(std::forward<T>(value))
    {}

    using storage_type::first;
    using typename storage_type::valref_type;
    using typename storage_type::const_valref_type;

    explicit tuple() = default;

    static constexpr int index = sizeof...(TArgs);

    typedef T element_type;

    // EXPERIMENTAL, though I think I'm wanting to keep it
    template <class F, class ...Args2>
    bool visit(F&& f, Args2&&...args)
    {
        return types::visitor::visit(internal::visit_tuple_functor{}, *this, f,
            std::forward<Args2>(args)...);
    }

    template <class F, class ...Args2>
    constexpr bool visit(F&& f, Args2&&...args) const
    {
        return types::visitor::visit(internal::visit_tuple_functor{}, *this, f,
            std::forward<Args2>(args)...);
    }
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


template<int index, typename... Types>
typename tuple_element<index, tuple<Types...> >::const_valref_type get(const tuple<Types...>& t)
{
    return internal::GetImpl<index, Types...>::value(t);
}

template<int index, typename... Types>
typename tuple_element<index, tuple<Types...> >::valref_type get(tuple<Types...>& t)
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
