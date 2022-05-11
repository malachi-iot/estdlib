/**
 * @file
 * chrono is rather platform specific at times, so dispatch out here
 * NOTE: When FreeRTOS is in play, it supercedes other bare metal and/or
 *       Arduino time API
 * TODO: Move all this into port/chrono.h
 */
#pragma once

#include "internal/platform.h"
#include "port/chrono.h"

// esp-idf native version coexists with the others
#if defined(ESTD_SDK_IDF)
#include "port/esp-idf/chrono.h"
#include "port/posix/chrono.h"
#endif

#if defined(ESTD_OS_FREERTOS)
#include "port/freertos/chrono.h"
#elif defined(ESTD_ARDUINO)
#include "port/arduino/chrono.h"
#elif defined(FEATURE_POSIX_CHRONO) || defined(WIN32)
#include "port/posix/chrono.h"
#else
#error Unsupported platform
#endif

// if we don't already have a std::chrono::steady_clock aliased...
#ifndef FEATURE_STD_CHRONO

// alias out steady_clock depending on system settings
namespace estd { namespace chrono {
#ifdef FEATURE_ESTD_FREERTOS_CHRONO
typedef freertos_clock steady_clock;
#else
#ifdef ATMEL_ASF
#include "port/asf/chrono.h"
typedef asf_clock steady_clock;
#elif defined(ESTD_ARDUINO) // finish from ATMEL_ASF
typedef arduino_clock steady_clock;
#endif
#endif

} }

#endif  // FEATURE_STD_CHRONO
