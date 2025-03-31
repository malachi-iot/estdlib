#pragma once

#include "internal/numeric.h"
#include "internal/utility.h"

namespace estd {

// Cheap and nasty copy/paste direct from
// https://en.cppreference.com/w/cpp/algorithm/accumulate
// TODO: Wrap these up in case they cascade out to blessed functions
template<class InputIt, class T>
ESTD_CPP_CONSTEXPR(14) T accumulate(InputIt first, InputIt last, T init)
{
    for (; first != last; ++first)
        init = std::move(init) + *first;

    return init;
}

template<class InputIt, class T, class BinaryOperation>
ESTD_CPP_CONSTEXPR(14) T accumulate(InputIt first, InputIt last, T init, BinaryOperation&& op)
{
    for (; first != last; ++first)
        init = op(std::move(init), *first);

    return init;
}

}