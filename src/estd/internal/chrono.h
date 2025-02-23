#pragma once

#include "fwd/chrono.h"
#include "chrono/features.h"
#include "chrono/duration.h"
#include "chrono/time_point.h"
#include "common_type.h"

namespace estd { namespace chrono {

namespace internal {

template <class TClock>
struct clock_traits
{
    template <class T>
    ESTD_CPP_CONSTEXPR_RET static T adjust_epoch(T t) { return t; }
};

}


}}
