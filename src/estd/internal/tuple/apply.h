#pragma once

#include "../tuple/get.h"
#include "../utility/sequence.h"

namespace estd {

namespace internal {

// 29MAR25 MB DEBT: Pretty sure we need to std::forward<Tuple>(t) around here
template <class F2, class Tuple, size_t... Is>
constexpr auto apply_impl(F2&& f, Tuple&& t, index_sequence<Is...>) ->
decltype (f(get<Is>(t)...))
{
    return f(get<Is>(t)...);
}


}

template <class F, class Tuple, class Seq = make_index_sequence<
    tuple_size<
        remove_reference_t<Tuple>
    >::value> >
constexpr auto apply(F&& f, Tuple&& t) ->
decltype (internal::apply_impl(std::forward<F>(f),
    std::forward<Tuple>(t),
    Seq {}))
{
    return internal::apply_impl(std::forward<F>(f),
        std::forward<Tuple>(t),
        Seq {});
}

}