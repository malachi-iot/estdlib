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
    typedef TickType_t rep;
    typedef internal::freertos_system_period period;
    typedef internal::estd_chrono::duration<rep, period> duration;
    typedef internal::estd_chrono::time_point<freertos_clock> time_point;

    static CONSTEXPR bool is_steady = true;

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
