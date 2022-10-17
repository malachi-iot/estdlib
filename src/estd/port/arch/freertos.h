#pragma once

#define ESTD_FREERTOS   // deprecated - do not use

// DEBT: Not the best place for feature flag defaults, but it will do

// One may not want to include FreeRTOS task.h all the time, which is a prereq for ../freertos/version.h to work
// Therefore one may inhibit the behavior with a feature flag, resulting in ESTD_OS_FREERTOS set to 1
// rather than FreeRTOS version.  One may manually include freertos/version.h to redefine ESTD_OS_FREERTOS
// DEBT: Document this in a central feature flag area
#ifndef FEATURE_ESTD_FREERTOS_VERSION
#define FEATURE_ESTD_FREERTOS_VERSION 1
#endif

#if FEATURE_ESTD_FREERTOS_VERSION
#include "../freertos/version.h"
#else
#define ESTD_OS_FREERTOS 1
#endif

#ifndef FEATURE_ESTD_FREERTOS_THREAD
#define FEATURE_ESTD_FREERTOS_THREAD 1
#endif