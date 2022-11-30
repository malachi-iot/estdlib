// Based on https://raspberrypi.github.io/pico-sdk-doxygen/group__sleep.html
// NOTE: On hold because regular std::system_clock and friends are present
#pragma once

#include "../../identify_platform.h"

#if ESTD_OS_TYPE == ESTD_RTOS_TYPE_FREERTOS
#include "../../freertos/chrono.h"
#endif

#include <chrono>

namespace estd {

namespace chrono {

namespace experimental {

struct pico_clock
{
    typedef uint64_t rep;
};

}

/*
 * No mappings for now due to ambiguity of feature availability
 * https://github.com/raspberrypi/pico-sdk/issues/1034
 * 
#if ESTD_OS_TYPE
// DEBT: This is just to get things compiling.  Although freertos_clock
// likely well qualifies as steady_clock, I'd rather #ifdef consuming code
// instead
// NOTE: It sure seems like std system_clock already depend
// on FreeRTOS since it dies in non FreeRTOS mode.  Steady clock seems to behave
typedef estd::chrono::freertos_clock steady_clock;
typedef estd::chrono::freertos_clock system_clock;
#else
typedef std::chrono::system_clock system_clock;
typedef std::chrono::steady_clock steady_clock;
#endif
*/

}

}