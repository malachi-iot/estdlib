#pragma once

// https://stackoverflow.com/questions/2166483/which-macro-to-wrap-mac-os-x-specific-code-in-c-c
// https://sourceforge.net/p/predef/wiki/OperatingSystems/

// Put little-to-no-dependency type code here, used for discovering platform
// and registering fundamental types/defines associated with said platform
// Included at the beginning of the platform.h feature discovery chain
// For more complex "always on" platform behaviors, utilize support_platform.h

// Putting aside for the moment what is truly the difference between RTOS and
// full OS... for now, if it calls itself an RTOS, we classify it as such.
#define ESTD_OS_TYPE_BARE 0
#define ESTD_OS_TYPE_RTOS 1
#define ESTD_OS_TYPE_FULL 1000

#define ESTD_RTOS_TYPE_FREERTOS     (ESTD_OS_TYPE_RTOS + 0)
#define ESTD_RTOS_TYPE_VDK          (ESTD_OS_TYPE_RTOS + 1)
#define ESTD_OS_TYPE_LINUX          (ESTD_OS_TYPE_FULL + 1)
#define ESTD_OS_TYPE_MACOS          (ESTD_OS_TYPE_FULL + 2)
#define ESTD_OS_TYPE_WINDOWS        (ESTD_OS_TYPE_FULL + 3)
// This is a catch all type for when we can't identify the particular kind of UNIX
#define ESTD_OS_TYPE_UNIX           (ESTD_OS_TYPE_FULL + 4)

#if defined(ARDUINO)
// DEBT: Arduino is more of a framework than an OS
#define ESTD_ARDUINO
#include "arch/arduino.h"
// DEBT: Arduino is not mutually exclusive to FreeRTOS
// NOTE: esp-idf doesn't define FREERTOS flag.  See below
#elif defined(FREERTOS)
#include "arch/freertos.h"
#define ESTD_OS_TYPE ESTD_RTOS_TYPE_FREERTOS
#elif (defined(__APPLE__) && defined(__MACH__))
#define ESTD_OS_MACOS
#define ESTD_OS_TYPE ESTD_OS_TYPE_FULL
#elif (defined(__unix__) || defined(__linux__))
#define ESTD_OS_UNIX
#ifdef __linux__
#define ESTD_OS_TYPE ESTD_OS_TYPE_LINUX
#else
#define ESTD_OS_TYPE ESTD_OS_TYPE_UNIX
#endif
#elif (defined(_WIN32) || defined(_WIN64))
#define ESTD_OS_WINDOWS
#define ESTD_OS_TYPE ESTD_OS_TYPE_WINDOWS
#endif

#if defined (__ADSPBLACKFIN__)
#define ESTD_MCU_BLACKFIN
// DEBT: It's possible to use blackfin without VDK, so test for that
#define ESTD_OS_TYPE ESTD_RTOS_TYPE_VDK
#elif defined (__AVR__)
#include "arch/avr.h"
#else
// investigate, see if we're stm32
#include "arch/stm32.h"

#ifndef ESTD_MCU_STM32
// TODO: Next up look for Atmel/Microchip ARM
#include "arch/atmelsam.h"
#endif

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

#if !defined(ESTD_OS_FREERTOS)
#include "arch/freertos.h"
#endif


#endif  // defined(ESP_PLATFORM)

#ifdef ESTD_OS_FREERTOS

#ifndef FEATURE_ESTD_NATIVE_CHRONO
#define FEATURE_ESTD_FREERTOS_CHRONO
#endif

#endif

// Identify depth of POSIX support
#include "posix.h"
