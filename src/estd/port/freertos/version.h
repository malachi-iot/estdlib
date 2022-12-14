// Builds FreeRTOS version # into ESTD_OS_FREERTOS
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Need to pull in task.h in order to acquire tskKERNEL_ macros
#if ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <FreeRTOS.h>
#include <task.h>
#endif

#include "../arch/version.h"

#ifdef ESTD_OS_FREERTOS

// See feature-flags.md 
// ESTD_OS_FREERTOS was specified by CMake
#if ESTD_OS_FREERTOS == 2 && FEATURE_ESTD_FREERTOS_VERSION
#undef ESTD_OS_FREERTOS
#else

#if ESTD_OS_FREERTOS == 1
#undef ESTD_OS_FREERTOS
#else
#error ESTD_OS_FREERTOS contains value other than 1
#endif
#if FEATURE_ESTD_FREERTOS_VERSION == 1
#warning Unexpected: redefining ESTD_OS_FREERTOS despite FEATURE_ESTD_FREERTOS_VERSION being enabled
#endif

#endif

#endif

#define ESTD_OS_FREERTOS    ESTD_BUILD_SEMVER(tskKERNEL_VERSION_MAJOR, tskKERNEL_VERSION_MINOR, tskKERNEL_VERSION_BUILD)

#ifdef __cplusplus
}
#endif
