#pragma once

#include "../internal/fwd/chrono.h"
#include "../internal/chrono/duration.hpp"
#include "../internal/chrono/operators.hpp"
#include "../internal/chrono/ratio.h"
#include "../internal/macro/push.h"

namespace estd { namespace chrono {

#if __cpp_constexpr
// lifted from https://en.cppreference.com/w/cpp/chrono/duration/abs
template <class Rep, class Period, class = estd::enable_if_t<
    estd::numeric_limits<Rep>::is_signed &&
    duration<Rep, Period>::min() < duration<Rep, Period>::zero()> >
constexpr duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return d >= d.zero() ? d : -d;
}
#else
// UNTESTED
// DEBT: I think even c++03 can check for min()/zero() at compile time, so if
// so we can merge this and above version
template <class Rep, class Period>
inline typename estd::enable_if<
    estd::numeric_limits<Rep>::is_signed, duration<Rep, Period> >::type
    abs(duration<Rep, Period> d)
{
    return d >= d.zero() ? d : -d;
}
#endif

namespace internal {

// DEBT: We'd like this to be c++03 friendly
#if __cplusplus >= 201103L

template <class Rep, class Period, typename estd::enable_if<
    estd::numeric_limits<Rep>::is_signed, bool>::type = true>
CONSTEXPR duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return chrono::abs(d);
}


template <class Rep, class Period, typename estd::enable_if<
    !estd::numeric_limits<Rep>::is_signed, bool>::type = true>
CONSTEXPR duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return d;
}

#endif


}

}}


#include "../internal/macro/pop.h"
