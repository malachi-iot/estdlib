#pragma once

#include "../ratio.h"

namespace estd {

namespace internal {

// embedded-oriented version has lower precision.  Deviates from standard
// Revisit this on a per-platform and
// option-selectable level
typedef uint32_t nano_rep;
typedef uint32_t micro_rep;
typedef uint32_t miilli_rep;
typedef uint16_t seconds_rep;
typedef uint16_t minutes_rep;

}

namespace chrono {

template<
        class Rep,
        class Period = estd::ratio<1>
>
class duration
{
    // confusingly, 'ticks' actually represents # of periods, not specifically
    // system ticks
    Rep ticks;

protected:
    template <class Rep2, class Period2>
    static Rep convert_from(const duration<Rep2, Period2>& d);

public:
    typedef Rep rep;
    typedef Period period;

    CONSTEXPR rep count() const { return ticks; }

    template <class Rep2>
    CONSTEXPR explicit duration(const Rep2& r) : ticks(r)
    {

    }

    template <class Rep2, class Period2>
    CONSTEXPR duration(const duration<Rep2, Period2>& d);
};


typedef duration<internal::nano_rep, nano> nanoseconds;
typedef duration<internal::micro_rep, micro> microseconds;
typedef duration<internal::miilli_rep, milli> milliseconds;
typedef duration<internal::seconds_rep> seconds;
typedef duration<internal::minutes_rep, ratio<60>> minutes;
typedef duration<internal::minutes_rep, ratio<3600>> hours;

}

}

#include "chrono.hpp"
