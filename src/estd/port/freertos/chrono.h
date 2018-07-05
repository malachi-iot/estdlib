#pragma once

#include <FreeRTOS.h>
#include "../chrono.h"

namespace estd {

// configTICK_RATE_HZ = ticks we expect to occur per second
// portTICK_PERIOD_MS = milliseconds we expect to occur per tick
// std:: tick period =
// "a tick period, where the tick period is a compile-time rational
// constant representing the number of seconds from one tick to the next."
namespace internal {

typedef ratio<1, configTICK_RATE_HZ> system_period;

}

struct steady_clock;

}
