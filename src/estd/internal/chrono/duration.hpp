#pragma once

#include "duration.h"

namespace estd { namespace chrono {

template<
    class Rep,
    class Period
    >
template<
    class Rep2,
    class Period2
    >
ESTD_CPP_CONSTEXPR_RET Rep duration<Rep, Period>::convert_from(const duration<Rep2, Period2>& d)
{
#ifdef __cpp_alias_templates
    typedef ratio_divide<Period2, Period> rd;
#else
    typedef detail::ratio_divide<Period2, Period> rd;
#endif

// So this isn't the answer but it's close
// 1) a warning would be much preferred
// 2) duration_cast should suppress this, but doesn't
#ifdef _FEATURE_CPP_STATIC_ASSERT
    static_assert (estd::numeric_limits<Rep>::digits >= estd::numeric_limits<Rep2>::digits,
                  "Precision loss");
#endif

    // FIX: Overly simplistic and going to overflow in some conditions
    // put into this helper method so that (perhaps) we can specialize/overload
    // to avoid that
    //return d.count() * Period2::num() * Period::den() / (Period2::den() * Period::num());
    // A little surprising that this statement doesn't issue compiler warnings about precision loss
    // even when Rep is much smaller of an integer type than Rep2
    return d.count() * rd::num / rd::den;
}

#ifdef FEATURE_STD_CHRONO
template<
    class Rep,
    class Period
    >
template<
    class Rep2,
    class Period2
    >
CONSTEXPR Rep duration<Rep, Period>::convert_from(const std::chrono::duration<Rep2, Period2>& d)
{
    typedef std::ratio_divide<Period2, Period> rd;

    // FIX: Overly simplistic and going to overflow in some conditions
    // put into this helper method so that (perhaps) we can specialize/overload
    // to avoid that
    //return d.count() * Period2::num() * Period::den() / (Period2::den() * Period::num());
    return d.count() * rd::num / rd::den;
}
#endif

template<
    class Rep,
    class Period
    >
template<
    class Rep2,
    class Period2
    >
#ifdef FEATURE_CPP_CONSTEXPR
constexpr
#endif
    duration<Rep, Period>::duration(const duration<Rep2, Period2>& d)
    : ticks(convert_from(d))
{
}


template <class ToDuration, class Rep, class Period>
ToDuration duration_cast(const duration<Rep, Period>& d)
{
    return ToDuration(d);
}



}}
