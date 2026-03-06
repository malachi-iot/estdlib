#pragma once

#include "duration.h"

namespace estd { namespace chrono {

namespace detail {

// DEBT: Consolidate with units::convert_from, if we can
template<class Traits>
template<class Traits2>
constexpr auto duration<Traits>::convert_from(const duration<Traits2>& d) -> rep
{
    using period2 = typename Traits2::period2;
#ifdef __cpp_alias_templates
    typedef ratio_divide<period2, period> rd;
#else
    typedef detail::ratio_divide<period2, period> rd;
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
template<class Traits>
template<class Rep2, class Period2>
constexpr auto duration<Traits>::convert_from(const std::chrono::duration<Rep2, Period2>& d) -> rep
{
    // DEBT: Duplication with estd::ratio flavor is a no no, but needed so far

    typedef std::ratio_divide<Period2, period> rd;

    // FIX: Overly simplistic and going to overflow in some conditions
    // put into this helper method so that (perhaps) we can specialize/overload
    // to avoid that
    //return d.count() * Period2::num() * Period::den() / (Period2::den() * Period::num());
    return d.count() * rd::num / rd::den;
}
#endif

}

template <class ToDuration, class Rep, class Period>
constexpr ToDuration duration_cast(const duration<Rep, Period>& d)
{
    return ToDuration(d, units::relaxed_narrow_t{});
}



}}
