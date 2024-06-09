#pragma once

#include "fwd.h"
#include "../raw/cstddef.h"
#include "../raw/type_traits.h"

#if __cpp_variadic_templates

namespace estd {

namespace internal {

template <size_t pos, class TIndices>
struct get_index;

template <size_t pos, typename T, T ...Is>
struct get_index_finder;

template <typename T, T I, T ...Is>
struct get_index_finder<0, T, I, Is...> : integral_constant<T, I>
{
};


template <size_t pos, typename T, T I, T ...Is>
struct get_index_finder<pos, T, I, Is...> :
        get_index_finder<pos - 1, T, Is...>
{
};



template <size_t pos, class T, T ...Is>
struct get_index<pos, variadic::values<T, Is...> > : get_index_finder<pos, T, Is...>
{
};

template <typename T, T ...Is>
struct value_sequence_single {};

template <typename T, T I>
struct value_sequence_single<T, I>
{
    static constexpr T single() { return I; }
};

template <typename T, T ...Values>
struct value_sequence_accessor
{
    using visitor = variadic::value_visitor<T, Values...>;

    template <class F, class ...TArgs>
    constexpr static int visit(F&& f, TArgs&&...args)
    {
        return visitor::visit(std::forward<F>(f), std::forward<TArgs>(args)...);
    }
};


}


namespace variadic {

// Since we can track pointers and references too, I prefer the name variadic::values
// rather than integer_sequence
template <typename T, T ...Is>
struct values :
    internal::value_sequence_single<T, Is...>,
    internal::value_sequence_accessor<T, Is...>
{
    static constexpr size_t size() { return sizeof...(Is); }

    template <size_t pos>
    using get = internal::get_index_finder<pos, T, Is...>;

    template <T I2>
    using prepend = values<T, I2, Is...>;

    template <T I2>
    using append = values<T, Is..., I2>;

    static constexpr T first() { return get<0>::value; }
};

#if __cpp_template_auto
namespace experimental {

template <auto T, decltype(T) ...Is>
struct v<T, Is...> : values<decltype(T), T, Is...> {};

}
#endif

}

// NOTE: Superset of regular integer_sequence, this one has get, append
// and prepend too
template <typename T, T ...Is>
using integer_sequence = variadic::values<T, Is...>;

template <size_t ...Is>
using index_sequence = integer_sequence<size_t, Is...>;


}

#endif
