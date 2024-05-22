#pragma once

#include "../fwd/chrono.h"
#include "duration.hpp"
#include "ratio.h"
#include "../macro/push.h"

namespace estd { namespace chrono {

// lifted from https://en.cppreference.com/w/cpp/chrono/duration/abs
template <class Rep, class Period, class = estd::enable_if_t<
    estd::numeric_limits<Rep>::is_signed &&
        duration<Rep, Period>::min() < duration<Rep, Period>::zero()> >
constexpr duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return d >= d.zero() ? d : -d;
}


template <class Rep, class Period, typename estd::enable_if_t<
    estd::numeric_limits<Rep>::is_signed &&
        duration<Rep, Period>::min() >= duration<Rep, Period>::zero()> >
constexpr duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return d;
}


template <class Rep, class Period, typename estd::enable_if_t<
    !estd::numeric_limits<Rep>::is_signed, bool> = true>
constexpr duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return d;
}


}}

#include "../macro/pop.h"
