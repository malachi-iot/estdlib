#pragma once

#include "identify_platform.h"

#ifdef ESTD_ARDUINO
#include "arduino/ostream.h"
#endif

#if LIB_PICO_STDIO
#include "rpi/pico/ostream.h"
#endif