#pragma once

// https://stackoverflow.com/questions/2166483/which-macro-to-wrap-mac-os-x-specific-code-in-c-c
// https://sourceforge.net/p/predef/wiki/OperatingSystems/

// Put little-to-no-dependency type code here, used for discovering platform
// and registering fundamental types/defines associated with said platform
// Included at the beginning of the platform.h feature discovery chain
// For more complex "always on" platform behaviors, utilize support_platform.h

#if defined(ARDUINO)
#define ESTD_ARDUINO
#include "arch/arduino.h"
#elif defined(FREERTOS)
#define ESTD_FREERTOS
#elif (defined (__APPLE__) && defined(__MACH__))
#define ESTD_MACOS
#elif (defined (__unix__) || defined(__linux__))
#define ESTD_UNIX
#elif (defined (__ADSPBLACKFIN__))
#define ESTD_MCU_BLACKFIN
#else
// investigate, see if we're stm32
#include "arch/stm32.h"
#endif

#if defined(ESTD_UNIX) || defined(ESTD_MACOS)
#define ESTD_POSIX
#endif

// ESP_PLATFORM always does freeRTOS for esp-idf
#if defined(ESP_PLATFORM) && defined(IDF_VER)
#include "arch/esp-idf.h"
#if !defined(ESTD_FREERTOS)
#define ESTD_FREERTOS
#endif
#endif

#ifdef ESTD_FREERTOS

#ifndef FEATURE_ESTD_NATIVE_CHRONO
#define FEATURE_ESTD_FREERTOS_CHRONO
#endif

#endif
