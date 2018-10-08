#pragma once

#include "esp_timer.h"

namespace estd { namespace chrono {

struct esp_clock
{
    typedef estd::internal::micro_rep rep;
    typedef micro period;
    typedef internal::estd_chrono::duration<rep, period> duration;
    typedef internal::estd_chrono::time_point<esp_clock> time_point;

    static CONSTEXPR bool is_steady = true;

    static time_point now()
    {
        return time_point(duration(esp_timer_get_time()));
    }
};

}}
