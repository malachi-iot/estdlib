#pragma once

#include "../fwd/tuple.h"
#include "apply.h"

namespace estd {

//template <class T, class Tuple>
//constexpr T make_from_tuple(Tuple&& t);

template <class T, class ...Args>
constexpr T make_from_tuple(tuple<Args...>&& t) noexcept
{
    return estd::apply([](Args&&...args)
    {
        return T(std::forward<Args>(args)...);
    }, std::forward<tuple<Args...>>(t));
}

// DEBT: Need to do decay as per https://en.cppreference.com/w/cpp/utility/tuple/make_tuple
template <class... Types>
constexpr tuple<Types...> make_tuple(Types&&... args) noexcept
{
    return tuple<Types...>(std::forward<Types>(args)...);
}

template <class... Types>
constexpr tuple<Types&&...> forward_as_tuple(Types&&... args) noexcept
{
    return tuple<Types&&...>(std::forward<Types>(args)...);
}


}
