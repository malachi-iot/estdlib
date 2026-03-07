#pragma once

#include "base.h"

// NOTE: This and 'bytes' are reference units.  Coincidentally, they both make
// unusual references because of their edge case behaviors.  For percent, it's
// unusual in that it is 0-100 based rather than traditional 0-1 based for things
// like seconds, kilometers, etc.

namespace estd { namespace internal { namespace units {

struct percent_tag {};

}}}

namespace estd { namespace units { inline namespace v1 {

// Represented as 0 through 100, not 0 through 1
template <class Rep, class Period = estd::ratio<1>, typename F = passthrough<Rep> >
using percent = unit<Rep, Period, internal::units::percent_tag, F>;

using namespace estd::literals::units_literals;

}}}

namespace estd { namespace internal { namespace units {

template <>
struct traits<percent_tag>
{
    static constexpr const char* name() { return "percent"; }
    // FIX: Under esp-idf, this gets consumed somehow.  Does not need escaping
    // under linux.
    static constexpr const char* abbrev() { return "%"; }
};

//inline namespace v1 {

// These feel not quite ready yet - i.e. what's the major upgrade from a
// special multiply?  we keep coming back to the oddball 0-100 behavior of percent
template <class Rep1, class Ratio1, class F1, class Traits2>
inline estd::units::detail::unit<Traits2> get_from_percent(
    const estd::units::percent<Rep1, Ratio1, F1>& p,
    estd::units::detail::unit<Traits2> min,
    estd::units::detail::unit<Traits2> max)
{
    using target_type = estd::units::detail::unit<Traits2>;
    const target_type range = max - min;

    // DEBT: Use common_type here
    auto v = range.count() * p.count();

    // DEBT: Perhaps inspect num vs 100 to see if we would compile-time
    // calc differently - though I bet compiler will do a handy job of this
    // already
    v *= Ratio1::num;
    v /= 100 * Ratio1::den;

    return target_type(v + min.count());
}

template <class Rep1, class Ratio1, class F1, class Traits2>
inline estd::units::detail::unit<Traits2> get_from_percent(
    const estd::units::percent<Rep1, Ratio1, F1>& p,
    estd::units::detail::unit<Traits2> max)
{
    return get_from_percent(p, estd::units::detail::unit<Traits2>(0), max);
}


}}} // estd::internal::units

namespace estd { inline namespace literals { inline namespace units_literals {

constexpr units::v1::percent<unsigned> operator ""_pct (unsigned long long int v)
{
    return units::v1::percent<unsigned>(v);
}

constexpr units::v1::percent<double> operator ""_pct (long double v)
{
    return units::v1::percent<double>(static_cast<double>(v));
}

}}}
