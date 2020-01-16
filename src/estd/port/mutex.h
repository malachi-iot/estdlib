#pragma once

#include "identify_platform.h"

#ifdef ESTD_FREERTOS
#include "freertos/mutex.h"
#elif defined(ESTD_MCU_BLACKFIN)
#include "vdk/mutex.h"
#endif
