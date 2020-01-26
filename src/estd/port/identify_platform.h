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
#endif

#if defined (__ADSPBLACKFIN__)
#define ESTD_MCU_BLACKFIN
#else
// investigate, see if we're stm32
#include "arch/stm32.h"

#ifndef ESTD_MCU_STM32
// TODO: Next up look for Atmel/Microchip ARM
#endif

#endif


#if defined(ESTD_UNIX) || defined(ESTD_MACOS)
#define ESTD_POSIX
#endif

// ESP_PLATFORM always assumes FreeRTOS.  Bare metal not supported
// at this time (no impediment, just haven't idendified a good way
// to specify bare metal or not)
#if defined(ESP_PLATFORM)

#define ESTD_MCU_ESPRESSIF

#if defined(IDF_VER)    // native esp-idf rtos sdk sets this
#include "arch/esp-idf.h"
#elif defined(ESP32)    // platformio does not set IDV_VER, but does set this
#include "arch/esp-idf.h"
#endif

#if !defined(ESTD_FREERTOS)
#define ESTD_FREERTOS
#endif


#endif

#ifdef ESTD_FREERTOS

#ifndef FEATURE_ESTD_NATIVE_CHRONO
#define FEATURE_ESTD_FREERTOS_CHRONO
#endif

#endif
