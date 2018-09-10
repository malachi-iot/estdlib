#pragma once

#include "../chrono.h"

namespace estd {
namespace chrono {

// compiles, untested
struct arduino_clock
{
    typedef estd::internal::milli_rep rep;
    // Arduino leans on millis() call for general case timing, so we report that
    // as system_period
    // "a tick period, where the tick period is a compile-time rational
    // constant representing the number of seconds from one tick to the next."
    typedef estd::milli period;

    typedef internal::estd_chrono::duration<rep, period> duration;
    typedef internal::estd_chrono::time_point<arduino_clock> time_point;

    static CONSTEXPR bool is_steady = true;

    static time_point now()
    {
        return time_point(duration(millis()));
    }
};

}
}
