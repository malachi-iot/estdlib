#pragma once

#include "../tuple/get.h"
#include "../utility/sequence.h"

namespace estd {

namespace internal {

template <class F2, class Tuple, size_t... Is>
constexpr auto apply_impl(F2&& f, Tuple&& t, index_sequence<Is...>) ->
decltype (f(get<Is>(std::forward<Tuple>(t))...))
{
    return f(get<Is>(std::forward<Tuple>(t))...);
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