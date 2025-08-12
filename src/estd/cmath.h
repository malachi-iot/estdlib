#pragma once

#include "internal/platform.h"

#if FEATURE_STD_CMATH
#include <cmath>
#else
#include <math.h>
#endif

#include "internal/macro/push.h"

namespace estd {

#if FEATURE_STD_CMATH
using std::abs;
using std::acos;
using std::cos;
using std::sin;
using std::pow;
using std::floor;
using std::round;
#else
using ::abs;
using ::acos;
using ::cos;
using ::sin;
using ::pow;
using ::floor;
using ::round;

// GCC, even through the C signatures, seems to auto resolve float vs double here.
// I am skeptical whether this applies during earlier GCC/AVR.  We will see
//inline double sin(double v) { return ::sin(v); }
//inline float sin(float v) { return ::sinf(v); }

#endif

// DEBT: Extremely crude raise-power function.  Improve by way of
// https://stackoverflow.com/questions/18581560/any-way-faster-than-pow-to-compute-an-integer-power-of-10-in-c

template <class T>
inline ESTD_CPP_CONSTEXPR(14) T pow(T base, unsigned exp)
{
    T v = base;
    while(--exp)    v *= base;
    return v;
}

}

#include "internal/macro/pop.h"
