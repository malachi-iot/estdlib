#pragma once

#include "internal/platform.h"
#include "port/chrono.h"

#if defined(ESTD_FREERTOS)
#include "port/freertos/chrono.h"
#elif defined(ESTD_ARDUINO)
#include "port/arduino/chrono.h"
#elif defined(ESTD_POSIX)
#include "port/posix/chrono.h"
#else
#error Unsupported platform
#endif
