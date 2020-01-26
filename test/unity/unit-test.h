#pragma once

// NOTE: math.h is necessary before including unity.h
#include <math.h>
#include "unity.h"

// TODO: Make ESP_IDF_TESTING something specified/deduced from
// native esp-idf testing rather than guessed based on it being arduino
// (doing this temporarily to bring up pio unity testing)
#if defined(ESP_PLATFORM) && !defined(ARDUINO)
#define ESP_IDF_TESTING
#endif