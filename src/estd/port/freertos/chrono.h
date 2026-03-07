#pragma once

extern "C" {

// esp32 and esp8266 put their includes within the freertos folder
#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <FreeRTOS.h>
#include <task.h>
#endif

}

#include "../chrono.h"

namespace estd {

namespace chrono {


// configTICK_RATE_HZ = ticks we expect to occur per second
// portTICK_PERIOD_MS = milliseconds we expect to occur per tick
// std:: tick period =
// "a tick period, where the tick period is a compile-time rational
// constant representing the number of seconds from one tick to the next."
namespace internal {

typedef estd_ratio::ratio<1, configTICK_RATE_HZ> freertos_system_period;

}

struct freertos_clock
{
    //typedef estd::chrono::internal::milli_rep rep;
    using rep = TickType_t;
    using period = ratio<1, configTICK_RATE_HZ>;
    template <class Rep>
    struct traits : duration_traits<Rep, period>
    {
        // DEBT: A little more permissive than maybe it should be.  Done in response to 
        // https://github.com/malachi-iot/estdlib/issues/177 since feeding 'milliseconds' into
        // freertos durations is just too convenient.
        static constexpr auto options =
            units::v1::detail::options::default_initialized |
            units::v1::detail::options::permissive;
    };

    // FIX: Our unit base isn't quite fully a substitute for duration.  Either make it
    // so or make a detail::duration which takes Traits
    using duration = detail::duration<traits<rep>>; 
    using time_point = internal::estd_chrono::time_point<freertos_clock>;

    static constexpr bool is_steady = true;

    static time_point now()
    {
        return time_point(duration(xTaskGetTickCount()));
    }
};


}

namespace freertos {

ESTD_CPP_CONSTEXPR_RET estd::chrono::freertos_clock::duration max_delay()
{
    return estd::chrono::freertos_clock::duration(portMAX_DELAY);
}

}

}
