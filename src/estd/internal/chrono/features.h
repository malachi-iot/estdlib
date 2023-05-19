#pragma once

#include "../platform.h"

// While developing, leave this on
#define FEATURE_ESTD_CHRONO

// Fully activating more advanced common_type/promoted_type fused
// duration conversions
#define FEATURE_ESTD_CHRONO_EXP


// FEATURE_STD_CHRONO_CORE includes durations, time_points and implicitly ratios
// FEATURE_STD_CHRONO_CLOCK includes system_clock and friends
#if (defined(FEATURE_POSIX_CHRONO) || defined(ESTD_SDK_IDF) || defined(LIB_PICO_STDLIB)) && !defined(FEATURE_ESTD_NATIVE_CHRONO) && __cplusplus >= 201103L
// DEBT: Doing this define here is the wrong spot - should be earlier in port/platform chain
#define FEATURE_STD_CHRONO 1
#define FEATURE_STD_CHRONO_CORE 1
#if !defined(LIB_PICO_STDLIB)
// As per https://github.com/raspberrypi/pico-sdk/issues/1034 we exclude rpi pico
#define FEATURE_STD_CHRONO_CLOCK 1
#endif
#include <chrono>
#endif

