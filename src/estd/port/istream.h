#pragma once

#include "identify_platform.h"

#ifdef ESTD_ARDUINO
#include "arduino/istream.h"
#elif defined(ESP_PLATFORM)
#include "esp-idf/config/istream.h"
#endif