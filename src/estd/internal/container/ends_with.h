#pragma once

#include "../macro/c++/const.h"

namespace estd { namespace internal {

/// @brief ends_with_n
/// @param lhs main string
/// @param rhs string we're compairing against (must be smaller than lhs)
/// @param lhs_count
/// @param rhs_count
/// @return
template <typename It1, typename It2>
inline ESTD_CPP_CONSTEXPR(14) bool ends_with_n(It1 lhs, It2 rhs, int lhs_count, int rhs_count)
{
    // DEBT: Using signed int here is fiddly, since incoming count likely won't be signed

    if(rhs_count > lhs_count) return false;

    const int start = lhs_count - rhs_count;
    const It1 lhs_end = lhs + lhs_count;
    lhs += start;

    while(lhs < lhs_end)
        if(*lhs++ != *rhs++)    return false;

    return true;
}

}}
