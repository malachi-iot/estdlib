#pragma once

#include "identify_platform.h"
#include "../internal/feature/std.h"

#ifdef ESTD_OS_FREERTOS
#include "freertos/mutex.h"
#elif defined(ESTD_MCU_BLACKFIN)
#include "vdk/mutex.h"
#elif defined(FEATURE_STD_MUTEX)
#include "std/mutex.h"
#endif
