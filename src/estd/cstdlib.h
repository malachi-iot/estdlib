#pragma once

#include "internal/platform.h"
#include "type_traits.h"    // DEBT: A lower dependency footprint here would be nice

#if FEATURE_STD_CSTDLIB
#include <cstdlib>
#else
#include "stdlib.h"

namespace std {

inline void abort() { return ::abort(); }

}
#endif

// DEBT: Put this elsewhere
#ifndef FEATURE_ESTD_STD_CSTDLIB_ALIAS
#define FEATURE_ESTD_STD_CSTDLIB_ALIAS FEATURE_STD_CSTDLIB
#endif

namespace estd {

#if FEATURE_ESTD_STD_CSTDLIB_ALIAS
using std::abs;
#else
template <class Numeric>
constexpr enable_if_t<is_arithmetic<Numeric>::value, Numeric> abs(Numeric n)
{
    // Presuming compiler optimization of things like 2's complement optimization
    // until such time we identify it's failing us
    return n < 0 ? -n : n;
}
#endif

}

