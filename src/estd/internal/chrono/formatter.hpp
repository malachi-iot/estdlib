/**
 * C++20 formatter helpers and friends as per
 * https://en.cppreference.com/w/cpp/utility/format/formatter
 */
// DEBT: A lot of chrono code is in port/chrono.h/hpp, would be better if it was instead here
#pragma once

#include "../fwd/chrono.h"
#include "../chrono/duration.h"
#include "../chrono/math.h"

#include "units.hpp"

#include "../macro/push.h"

namespace estd { namespace chrono {

namespace internal {
    
// NOTE: Stand-ins for sys_time, sys_days
template<class Duration, class Clock>
using clock_time = chrono::time_point<Clock, Duration>;

template <class Clock>
using clock_days = clock_time<chrono::days, Clock>;

// As per
// https://en.cppreference.com/w/cpp/chrono/year_month_day example
// system_clock epoch is calculated into days(), months(), years()
template <class Clock>
class year_month_day
{
    using sys_days = clock_days<Clock>;
    using clock_traits = chrono::internal::clock_traits<Clock>;

    sys_days days_;

public:
    // FIX: Not working yet due to difficulty of calculation
    chrono::day days() const NOEXCEPT
    {
        chrono::months m = days_.time_since_epoch();
        chrono::days d = days_.time_since_epoch() - m;
        return clock_traits::adjust_epoch(chrono::day(d.count() + 1));
    }

    //constexpr
    chrono::month month() const NOEXCEPT
    {
        chrono::years y{days_.time_since_epoch(), units::relaxed_narrow_t{}};
        chrono::days d{y, units::relaxed_narrow_t{}};
        chrono::months m(days_.time_since_epoch() - d, units::relaxed_narrow_t{});
        return clock_traits::adjust_epoch(chrono::month(m.count() + 1));
    }

    //constexpr
    chrono::year year() const NOEXCEPT
    {
        chrono::years y(days_.time_since_epoch(), units::relaxed_narrow_t{});
        return clock_traits::adjust_epoch(chrono::year(y.count()));
    }

    constexpr year_month_day(const sys_days& dp) NOEXCEPT :
        days_{dp}
    {
    }
};

template <class TClock>
class month_day_last
{
    const chrono::month month_;

public:
    explicit CONSTEXPR month_day_last(const chrono::month& m) NOEXCEPT : month_{m}
    {
    }

    constexpr chrono::month month() const { return month_; }

    constexpr bool ok() const { return month_.ok(); }
};


template <class Clock>
class year_month_day_last
{
    const chrono::year year_;
    const chrono::month month_;

public:
    constexpr year_month_day_last(const chrono::year& y,
        const month_day_last<Clock>& mdl) NOEXCEPT :
        year_{y},
        month_{mdl.month()}
    {}

    constexpr chrono::year year() const { return year_; }

    inline chrono::day day() const
    {
        switch(month_.value())
        {
            case 2: return year_.is_leap() ? day(29) : day(28);

            case 4: return day(30);
            case 6: return day(30);
            case 11: return day(30);

            default:    return day(31);
        }
    }

    constexpr bool ok() const NOEXCEPT
    {
        return month_.ok() && year_.ok();
    }
};


template <>
class year_month_day<void>
{
    chrono::year year_;
    chrono::month month_;
    chrono::day day_;

public:
    constexpr year_month_day(
        const chrono::year& y,
        const chrono::month& m,
        const chrono::day& d) : year_{y}, month_{m}, day_{d}
    {}

    constexpr bool ok() const NOEXCEPT
    {
        return day_.ok() && month_.ok() && year_.ok();
    }

    constexpr chrono::day day() const NOEXCEPT { return day_; }
    constexpr chrono::month month() const NOEXCEPT { return month_; }
    constexpr chrono::year year() const NOEXCEPT { return year_; }
};

}

// DEBT: Optimize with modulo instead of brute force subtracting (which will cascade out into
// multiplies and divides)
template <class Duration>
class hh_mm_ss
{
    typedef typename Duration::rep rep;
    typedef typename Duration::period period;
    typedef chrono::duration<rep, period> duration_type;

    const duration_type value;

    typedef estd::is_signed<rep> is_signed;

    typedef duration<uint8_t, chrono::hours::period> hours_type;
    typedef duration<uint8_t, chrono::minutes::period> minutes_type;
    typedef duration<rep, chrono::seconds::period> seconds_type;

    constexpr duration_type _abs() const
    {
        return chrono::abs(value);
    }

public:
    explicit constexpr hh_mm_ss(Duration duration) : value(duration) {}

    // NOTE: Purposely leaving this raw instead of computing a common type against seconds
    // because we want to leave 'rep' in a lower-precision state for embedded systems unless
    // explicitly specified otherwise by Duration
    typedef Duration precision;

    constexpr bool is_negative() const NOEXCEPT
    {
        return is_signed::value ? value.count() < 0 : false;
    }

    constexpr hours_type hours() const NOEXCEPT
    {
        return hours_type{_abs(), units::relaxed_narrow_t{}};
    }

    constexpr minutes_type minutes() const NOEXCEPT
    {
        return minutes_type{_abs() - hours(), units::relaxed_narrow_t{}};
    }

    inline seconds_type seconds() const NOEXCEPT
    {
        duration_type a = _abs();
        auto v = a - minutes_type{a, units::relaxed_narrow_t{}};
        return seconds_type{v, units::relaxed_narrow_t{}};
    }

    inline precision subseconds() const NOEXCEPT
    {
        duration_type a = _abs();
        return precision{a - seconds_type{a, units::relaxed_narrow_t{}}};
    }

    constexpr explicit operator precision() const NOEXCEPT { return value; }
    constexpr precision to_duration() const NOEXCEPT { return value; }
};

}}

#include "../macro/pop.h"
