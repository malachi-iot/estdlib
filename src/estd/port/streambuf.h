#pragma once

#ifdef ESTD_POSIX
// Not ready quite yet
#include "posix/streambuf.h"
#elif defined(ESP_OPEN_RTOS)
#elif defined(__MBED__)
#include <drivers/Stream.h>
// looks like a collision
// on the F() macro, so undef it first
#ifdef F
#define F_DEFINED
#undef F
#endif
#include <drivers/Serial.h>
// redefine F here.  Kludgey for sure.  There will be situations where
// F wants to be what Serial.h set it to
#ifdef F_DEFINED
//#include "noduino_F.h"
#undef F_DEFINED
#endif

#elif defined(ARDUINO)
#include <Arduino.h>
#endif
