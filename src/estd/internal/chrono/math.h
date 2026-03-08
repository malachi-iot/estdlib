#pragma once

#include "../fwd/chrono.h"
#include "../units/operators.hpp"
#include "duration.hpp"
#include "ratio.h"
#include "../macro/push.h"

namespace estd { namespace chrono { namespace detail {

// lifted from https://en.cppreference.com/w/cpp/chrono/duration/abs
template <class Traits, class = estd::enable_if_t<
    estd::numeric_limits<typename Traits::rep>::is_signed &&
        duration<Traits>::min() < duration<Traits>::zero()> >
constexpr duration<Traits> abs(duration<Traits> d)
{
    return d >= d.zero() ? d : -d;
}


template <class Traits, typename estd::enable_if_t<
    estd::numeric_limits<typename Traits::rep>::is_signed &&
        duration<Traits>::min() >= duration<Traits>::zero()> >
constexpr duration<Traits> abs(duration<Traits> d)
{
    return d;
}


template <class Traits, typename estd::enable_if_t<
    !estd::numeric_limits<typename Traits::rep>::is_signed, bool> = true>
constexpr duration<Traits> abs(duration<Traits> d)
{
    return d;
}

}

using detail::abs;

}}

#include "../macro/pop.h"
