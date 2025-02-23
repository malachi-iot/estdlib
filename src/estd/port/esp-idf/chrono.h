#pragma once

#include <esp_timer.h>
#include <rtc.h>

#include "../../internal/chrono/duration.h"
#include "../../internal/chrono/time_point.h"

namespace estd { namespace chrono {

namespace internal {

template <typename Rep>
struct esp_clock
{
    using rep = Rep;
    using period = micro;
    using duration = internal::estd_chrono::duration<rep, period>;

    static constexpr bool is_steady = true;
};

}

struct esp_clock : internal::esp_clock<int64_t>
{
    using time_point = internal::estd_chrono::time_point<esp_clock>;

    static time_point now()
    {
        return time_point(duration(esp_timer_get_time()));
    }
};


struct esp_rtc_clock : internal::esp_clock<uint64_t>
{
    using time_point = internal::estd_chrono::time_point<esp_rtc_clock>;

    static time_point now()
    {
        return time_point(duration(esp_rtc_get_time_us()));
    }
};


}}
