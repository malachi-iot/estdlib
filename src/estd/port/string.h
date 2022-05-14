#pragma once

#ifdef ARDUINO
#include "arduino/string.h"
#else

#endif

// NOTE: Always include POSIX one.  It will make its own decision
// whether POSIX is really present
#include "posix/string.h"