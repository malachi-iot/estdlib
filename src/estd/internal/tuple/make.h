#pragma once

#include "../fwd/tuple.h"
#include "apply.h"

namespace estd {

//template <class T, class Tuple>
//constexpr T make_from_tuple(Tuple&& t);

template <class T, class ...Args>
constexpr T make_from_tuple(tuple<Args...>&& t)
{
    return apply([](Args&&...args)
    {
        return T(std::forward<Args>(args)...);
    }, std::forward<tuple<Args...>>(t));
}

template <class... Types>
constexpr tuple<Types...> make_tuple(Types&&... args )
{
    return tuple<Types...>(std::forward<Types>(args)...);
}


}