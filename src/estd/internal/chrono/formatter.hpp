/**
 * C++20 formatter helpers and friends as per
 * https://en.cppreference.com/w/cpp/utility/format/formatter
 */
// DEBT: A lot of chrono code is in port/chrono.h/hpp, would be better if it was instead here
#pragma once

#include "../fwd/chrono.h"
#include "../../port/chrono.hpp"

#include "units.hpp"

namespace estd { namespace chrono {

namespace internal {

template <class TClock>
class year_month_day
{
    typedef clock_days<TClock> sys_days;

    sys_days days_;

public:
    chrono::day days() const NOEXCEPT
    {
        chrono::months m = days_.time_since_epoch();
        chrono::days d = days_.time_since_epoch() - m;
        return chrono::day(d.count() + 1);
    }

    //constexpr
    chrono::month month() const NOEXCEPT
    {
        chrono::years y = days_.time_since_epoch();
        chrono::days d{y};
        chrono::months m = days_.time_since_epoch() - d;
        return chrono::month(m.count() + 1);
    }

    //constexpr
    chrono::year year() const NOEXCEPT
    {
        chrono::years y = days_.time_since_epoch();
        return chrono::year(y.count() + 1970);
    }

    constexpr year_month_day(const sys_days& dp) NOEXCEPT :
        days_{dp}
    {
    }
};

}

// DEBT: Optimize with modulo instead of brute force subtracting (which will cascade out into
// multiplies and divides)
// DEBT:
template <class Duration>
class hh_mm_ss
{
    const Duration value;

    typedef estd::is_signed<typename Duration::rep> is_signed;

    typedef duration<uint8_t, chrono::hours::period> hours_type;
    typedef duration<uint8_t, chrono::minutes::period> minutes_type;
    typedef duration<typename Duration::rep, chrono::seconds::period> seconds_type;

    constexpr Duration _abs() const
    {
        return chrono::internal::abs(value);
    }

public:
    hh_mm_ss(Duration duration) : value(duration) {}

    typedef Duration precision;

    constexpr bool is_negative() const NOEXCEPT
    {
        return is_signed::value ? value.count() < 0 : false;
    }

    constexpr hours_type hours() const NOEXCEPT
    {
        return hours_type{_abs()};
    }

    constexpr minutes_type minutes() const NOEXCEPT
    {
        return minutes_type{_abs() - hours()};
    }

    inline seconds_type seconds() const NOEXCEPT
    {
        Duration a = _abs();
        return seconds_type{a - minutes_type{a}};
    }

    inline precision subseconds() const NOEXCEPT
    {
        Duration a = _abs();
        return precision{a - seconds_type{a}};
    }

    constexpr explicit operator precision() const NOEXCEPT { return value; }
    constexpr precision to_duration() const NOEXCEPT { return value; }
};

}}