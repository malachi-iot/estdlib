/**
 *  @file
 *  FEATURE_ESTD_THREADING means actual multithreading is available
 *  Otherwise, just the basics like sleep_for etc are available
 */
#pragma once

#include "identify_platform.h"

// NOTE: sleep_for collision seems to be present for asf + Arduino,
// but so far it's compiling OK
#if defined(ATMEL_ASF) && !defined(FEATURE_ESTD_FREERTOS_CHRONO)
#include "asf/thread.h"
#endif

#define FEATURE_ESTD_THREADING

#ifdef FEATURE_ESTD_FREERTOS_CHRONO
#include "freertos/thread.h"
#elif defined(ESTD_ARDUINO)
#undef FEATURE_ESTD_THREADING
#include "arduino/thread.h"
#elif defined(ESTD_POSIX)
#include "posix/thread.h"
#elif defined(ESTD_MCU_BLACKFIN)
#include "vdk/thread.h"
#else
// indicating threading API so as to indicate not even sleep_for etc can
// be deduced
#warning "Couldn't find supported platform for threading API"
#endif
