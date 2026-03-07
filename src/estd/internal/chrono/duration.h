#pragma once

#include "features.h"
#include "../../ratio.h"
#include "../chrono.h"
#include "../fwd/chrono.h"
#include "../units/base.h"

#include "../macro/push.h"

#if FEATURE_STD_CHRONO
#include <chrono>
#endif

namespace estd { namespace chrono {

#ifdef FEATURE_ESTD_CHRONO

namespace internal {

// DEBT: Move this tag out to 'detail'
struct seconds_tag {};

// DEBT: Strongly consider reworking/infusing this into duration_traits
template <class Rep>
struct duration_values
{
    static constexpr Rep zero() { return Rep(0); }
    static constexpr Rep min() { return estd::numeric_limits<Rep>::min(); }
    static constexpr Rep max() { return estd::numeric_limits<Rep>::max(); }
};

}

template <class Rep, class Period>
struct duration_traits : estd::units::v1::detail::traits<Rep, Period, internal::seconds_tag>
{
    static constexpr units::detail::options options =
        units::v1::detail::options::default_initialized;

#if FEATURE_STD_CHRONO_CORE
    using std_period = std::ratio<Period::num, Period::den>;
    using std_duration = std::chrono::duration<Rep, std_period>;
#endif
};


namespace detail {

// TODO: Give mixins a try here to reduce footprint of duration.  Note that will bulk
// up c++ error surface when things go wrong
template <class Traits>
class duration : public units::v1::detail::unit<Traits>
{
    using base_type = units::v1::detail::unit<Traits>;
    using base_type::rep_;

public:
    using typename base_type::rep;
    using typename base_type::period;

protected:
#if FEATURE_STD_CHRONO_CORE || FEATURE_STD_CHRONO
    template <class Rep2, class Period2>
    static constexpr rep convert_from(const std::chrono::duration<Rep2, Period2>& d);
#endif

    using duration_values = internal::duration_values<rep>;

public:
    constexpr duration() = default;

    template <class Rep2>
    constexpr explicit duration(const Rep2& r) :
        base_type(r)
    {}

    template <class Traits2>
    constexpr duration(const duration<Traits2>& d) :  // NOLINT
        base_type(d)
    {}

    template <class Traits2>
    constexpr duration(const duration<Traits2>& d, units::relaxed_narrow_t) :  // NOLINT
        base_type(d, units::relaxed_narrow_t{})
    {}

#if FEATURE_STD_CHRONO_CORE || FEATURE_STD_CHRONO
    template <class Rep2, class Period2>
    constexpr duration(const std::chrono::duration<Rep2, Period2>& d) : // NOLINT
        base_type(convert_from(d))
    {}

    using std_period_type = std::ratio<period::num, period::den>;

    constexpr operator std::chrono::duration<rep, std_period_type>() const // NOLINT
    {
        return std::chrono::duration<rep, std_period_type>(rep_);
    }
#endif

    ESTD_CPP_CONSTEXPR(17) duration& operator+=(const duration& d)
    {
        rep_ += d.count();
        return *this;
    }


    ESTD_CPP_CONSTEXPR(17) duration& operator-=(const duration& d)
    {
        rep_ -= d.count();
        return *this;
    }

    ESTD_CPP_CONSTEXPR(17) duration& operator*=(const rep& r)
    {
        rep_ *= r;
        return *this;
    }

    ESTD_CPP_CONSTEXPR(17) duration& operator/=(const rep& r)
    {
        rep_ /= r;
        return *this;
    }

    constexpr duration operator-() const
    {
        static_assert(numeric_limits<rep>::is_signed, "operator -() requires a signed Rep type");

        return duration(-rep_);
    }

    constexpr duration operator+() const { return *this; }

    static constexpr duration min() { return duration(duration_values::min()); }
    static constexpr duration max() { return duration(duration_values::max()); }
    static constexpr duration zero() { return duration(duration_values::zero()); }
};

}

#endif

// These lower-precision ones are available even during non-FEATURE_ESTD_CHRONO
typedef internal::estd_chrono::duration<internal::nano_rep, nano> nanoseconds;
typedef internal::estd_chrono::duration<internal::micro_rep, micro> microseconds;
typedef internal::estd_chrono::duration<internal::milli_rep, milli> milliseconds;
typedef internal::estd_chrono::duration<internal::seconds_rep> seconds;
typedef internal::estd_chrono::duration<internal::minutes_rep, ratio<60> > minutes;
typedef internal::estd_chrono::duration<internal::hours_rep, ratio<3600> > hours;

// NOTE: AVR compiler requires this long typecast.  Doesn't hurt anything (though it's
// a bit ugly)
typedef internal::estd_chrono::duration<internal::days_rep, ratio<(long)3600 * 24> > days;
typedef internal::estd_chrono::duration<internal::weeks_rep, ratio<(long)3600 * 24 * 7> > weeks;

// "Each of the predefined duration types days, weeks, months and years covers a range
//  of at least ±40000 years. years is equal to 365.2425 days (the average length of a
//  Gregorian year). months is equal to 30.436875 days (exactly 1/12 of years)."
// https://en.cppreference.com/w/cpp/chrono/duration
typedef internal::estd_chrono::duration<internal::months_rep, ratio<(long)2629746> > months;
typedef internal::estd_chrono::duration<internal::years_rep, ratio<(long)31556952> > years;


}}

#include "../macro/pop.h"

