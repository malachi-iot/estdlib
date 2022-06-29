#pragma once

#include "fwd/chrono.h"
#include "common_type.h"

// While developing, leave this on
#define FEATURE_ESTD_CHRONO

// Fully activating more advanced common_type/promoted_type fused
// duration conversions
#define FEATURE_ESTD_CHRONO_EXP


namespace estd { namespace chrono {

namespace internal {

template <class TClock>
struct clock_traits
{
    template <class T>
    constexpr static T adjust_epoch(T t) { return t; }
};

// Use this to turn on incomplete estd::chrono support namespace
// there's a lot that goes into a healthy, functioning chrono namespace
// so we default to using std::chrono
#ifdef FEATURE_ESTD_CHRONO
namespace estd_ratio = estd;
namespace estd_chrono = estd::chrono;
#elif FEATURE_STD_CHRONO
namespace estd_ratio = std;
namespace estd_chrono = std::chrono;
#else
#warning Invalid configuration, neither std or estd chrono fully activated
#endif

}


}}