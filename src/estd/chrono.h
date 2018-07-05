#pragma once

#if defined(FEATURE_ESTD_FREERTOS) || defined(FREERTOS)
#include "port/freertos/chrono.h"
#else
#error Unsupported platform
#endif
