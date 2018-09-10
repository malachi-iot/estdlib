#pragma once

#include "identify_platform.h"

#if defined(ATMEL_ASF) && !defined(FEATURE_ESTD_FREERTOS_CHRONO)
#include "asf/thread.h"
#endif

#ifdef FEATURE_ESTD_FREERTOS_CHRONO
#include "freertos/thread.h"
#elif defined(ESTD_ARDUINO)
#include "arduino/thread.h"
#elif defined(ESTD_POSIX)
#include "posix/thread.h"
#endif
