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

template <class Duration>
class hh_mm_ss
{
    Duration value;

    typedef duration<uint8_t, chrono::hours::period> hours_type;
    typedef duration<uint8_t, chrono::minutes::period> minutes_type;
    typedef duration<typename Duration::rep, chrono::seconds::period> seconds_type;

public:
    hh_mm_ss(Duration duration) : value(duration) {}

    typedef Duration precision;

    constexpr hours_type hours() const
    {
        return hours_type{value};
    }

    constexpr minutes_type minutes() const
    {
        return minutes_type{value - hours()};
    }

    constexpr seconds_type seconds() const
    {
        return seconds_type{value - minutes_type{value}};
    }

    constexpr precision subseconds() const
    {
        return precision{value - seconds_type{value}};
    }
};

}}