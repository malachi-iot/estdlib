#pragma once

#include "../raw/cstddef.h"
#include "../raw/type_traits.h"

#if __cpp_variadic_templates

namespace estd {

namespace variadic {

template <typename T, T ...Is>
struct value_sequence;

}

namespace internal {

template <int pos, class TIndices>
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



template <int pos, class T, T ...Is>
struct get_index<pos, variadic::value_sequence<T, Is...> > : get_index_finder<pos, T, Is...>
{
};

template <typename T, T ...Is>
struct value_sequence_single {};

template <typename T, T I>
struct value_sequence_single<T, I>
{
    static constexpr T single() { return I; }
};


}


namespace variadic {

// Since we can track pointers and references too, I prefer the name value_sequence
// rather than integer sequence
template <typename T, T ...Is>
struct value_sequence : internal::value_sequence_single<T, Is...>
{
    static constexpr size_t size() { return sizeof...(Is); }

    template <int pos>
    using get = internal::get_index_finder<pos, T, Is...>;

    template <int I2>
    using prepend = value_sequence<T, I2, Is...>;

    template <int I2>
    using append = value_sequence<T, Is..., I2>;

    static constexpr T first() { return get<0>::value; }
};

}

// NOTE: Superset of regular integer_sequence, this one has get, append
// and prepend too
template <typename T, T ...Is>
using integer_sequence = variadic::value_sequence<T, Is...>;

template <size_t ...Is>
using index_sequence = integer_sequence<size_t, Is...>;


}

#endif
