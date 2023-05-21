#pragma once

#include "../platform.h"

// By default we use our own implementation of chrono.  If this is turned off,
// we do minimal internal aliasing of std chrono to estd and naturally our own
// chrono is disabled.  Used this for bringup, and probably should remove the flag
// entirely
// NOTE: This is not well tested and probably
// broken, so if in doubt, leave this alone!
#ifndef FEATURE_ESTD_CHRONO
#define FEATURE_ESTD_CHRONO 1
#endif

// FEATURE_STD_CHRONO_CORE includes durations, time_points and implicitly ratios
// FEATURE_STD_CHRONO_CLOCK includes system_clock and friends
#if (defined(FEATURE_POSIX_CHRONO) || defined(ESTD_SDK_IDF) || defined(LIB_PICO_STDLIB)) && !defined(FEATURE_ESTD_NATIVE_CHRONO) && __cplusplus >= 201103L
// DEBT: Doing this define here is the wrong spot - should be earlier in port/platform chain
#define FEATURE_STD_CHRONO_CORE 1
#if !defined(LIB_PICO_STDLIB)
// As per https://github.com/raspberrypi/pico-sdk/issues/1034 we exclude rpi pico
#define FEATURE_STD_CHRONO_CLOCK 1
#endif
#include <chrono>
#endif

