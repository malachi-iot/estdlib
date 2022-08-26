extern "C" {

#if ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <task.h>
#endif

}

#include "version.h"

#define ESTD_OS_FREERTOS    ESTD_BUILD_SHORT_VER(tskKERNEL_VERSION_MAJOR, tskKERNEL_VERSION_MINOR, tskKERNEL_VERSION_BUILD)

// DEBT: Not the best place for feature flag defaults, but it will do

#ifndef FEATURE_ESTD_FREERTOS_THREAD
#define FEATURE_ESTD_FREERTOS_THREAD 1
#endif