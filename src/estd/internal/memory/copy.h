#pragma once

#include "move.h"

namespace estd {

#if FEATURE_ESTD_STD_MEMORY_ALIAS
using std::uninitialized_copy;
using std::uninitialized_copy_n;
#else
// Adapted from https://en.cppreference.com/cpp/memory/uninitialized_copy
template<class InputIt, class ForwardIt>
ESTD_CPP_CONSTEXPR(14) ForwardIt uninitialized_copy(InputIt first, InputIt last,
    ForwardIt d_first)
{
    return internal::uninitialized_ops<InputIt, ForwardIt>::
        move_or_copy(first, last, d_first, false_type{});
}

template<class InputIt, class Size, class ForwardIt>
ESTD_CPP_CONSTEXPR(14) ForwardIt uninitialized_copy_n(InputIt first, Size count,
    ForwardIt d_first)
{
    return internal::uninitialized_ops<InputIt, ForwardIt>::
        move_or_copy_n(first, count, d_first, false_type{});
}
#endif

}