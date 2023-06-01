#pragma once

#include "../raw/cstddef.h"
#include "../raw/type_traits.h"

#if __cpp_variadic_templates

namespace estd {

// DEBT: Continue to combine this with already-existing integer_sequence
template <typename T, T ...Is>
struct integer_sequence;

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
struct get_index<pos, integer_sequence<T, Is...> > : get_index_finder<pos, T, Is...>
{
};


}

// NOTE: Superset of regular integer_sequence, this one has get, append
// and prepend too
template <typename T, T ...Is>
struct integer_sequence
{
    //static constexpr int value = I;
    //static constexpr int position = sizeof...(Is);
    static constexpr size_t size() { return sizeof...(Is); }

    template <int pos>
    using get = internal::get_index_finder<pos, T, Is...>;

    template <int I2>
    using prepend = integer_sequence<T, I2, Is...>;

    template <int I2>
    using append = integer_sequence<T, Is..., I2>;
};

template <size_t ...Is>
using index_sequence = integer_sequence<size_t, Is...>;


}

#endif
