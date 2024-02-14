#pragma once

#include "../platform.h"

#if FEATURE_ESTD_DRAGONBOX
// DEBT: Coud use a better location
// DEBT: A better location would be in originating repo (PR?)
namespace jkj { namespace dragonbox {

template <class T>
struct default_float_traits;

}}
#endif

namespace estd {

template <class T>
struct numeric_limits
{
    static CONSTEXPR bool is_specialized = false;
    static CONSTEXPR bool is_integer = false;
    static CONSTEXPR bool is_iec559 = false;
};

}
