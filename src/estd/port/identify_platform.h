#pragma once

// https://stackoverflow.com/questions/2166483/which-macro-to-wrap-mac-os-x-specific-code-in-c-c
// https://sourceforge.net/p/predef/wiki/OperatingSystems/

#if defined(ARDUINO)
#define ESTD_ARDUINO
#elif defined(FREERTOS)
#define ESTD_FREERTOS
#elif (defined (__APPLE__) && defined(__MACH__))
#define ESTD_MACOS
#elif (defined (__unix__) || defined(__linux__))
#define ESTD_UNIX
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
