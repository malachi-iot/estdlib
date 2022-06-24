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
    constexpr chrono::month month() const NOEXCEPT
    {
        return chrono::month(7);
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

}}