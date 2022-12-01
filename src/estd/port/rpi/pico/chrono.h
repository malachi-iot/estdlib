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

typedef internal::estd_chrono::duration<uint64_t, estd::micro> pico_duration_us;

// Due to aforementioned ambiguity, implementing our own simplistic version
// of steady/high_resolution clock.  Probably one of those actually does
// this already
struct pico_clock
{
    typedef pico_duration_us duration;
    typedef internal::estd_chrono::time_point<pico_clock> time_point;
    typedef duration::rep rep;
    typedef duration::period period;

    static constexpr bool is_steady = true;

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

// spec deviation - 'this_thread' seemed inappropriate., since we may not be in multithreaded environment
// though not named as such, this is experimental
namespace this_core {

inline void sleep_for(const estd::chrono::experimental::pico_duration_us& sleep_duration)
{
    sleep_us(sleep_duration.count());
}


template <class TDuration>
inline void sleep_until(const estd::chrono::time_point<estd::chrono::experimental::pico_clock, TDuration>& sleep_time)
{
    estd::chrono::experimental::pico_duration_us converted(sleep_time.time_since_epoch());
    absolute_time_t target = delayed_by_us(nil_time, converted.count());
    ::sleep_until(target);
}

}

}