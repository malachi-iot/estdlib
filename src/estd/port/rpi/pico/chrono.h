// Based on https://raspberrypi.github.io/pico-sdk-doxygen/group__sleep.html
// NOTE: On hold because regular std::system_clock and friends are present
#pragma once

#include "../../identify_platform.h"
#include "../../chrono.h"

#include <pico/time.h>

/*
 Temporarily disabling this mapping until we sort out general std chrono
 behavior for rpi pico.  Naturally, freertos_clock is still available
 separately
#if ESTD_OS_TYPE == ESTD_RTOS_TYPE_FREERTOS
#include "../../freertos/chrono.h"
#endif
*/

#include <chrono>

namespace estd {

namespace chrono {

namespace experimental {

typedef internal::estd_chrono::duration<uint64_t, estd::micro> pico_duration;

// Due to aforementioned ambiguity, implementing our own simplistic version
// of steady/high_resolution clock.  Probably one of those actually does
// this already
struct pico_clock
{
    typedef uint64_t rep;
    typedef estd::micro period;
    typedef internal::estd_chrono::duration<rep, period> duration;
    typedef internal::estd_chrono::time_point<pico_clock> time_point;

    static CONSTEXPR bool is_steady = true;

    static time_point now()
    {
        return time_point(duration(time_us_64()));
    }
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

// spec deviation, since we may not be in multithreaded environment
namespace this_core {

template< class Rep, class Period >
void sleep_for( const chrono::duration<Rep, Period>& sleep_duration )
{
    typedef estd::chrono::experimental::pico_duration duration;

    auto count =
            estd::chrono::duration_cast<duration>(sleep_duration).count();

    sleep_us(count);
}

}

}