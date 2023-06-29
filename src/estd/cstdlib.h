#pragma once

#include "internal/platform.h"

#if FEATURE_STD_CSTDLIB
#include <cstdlib>
#else
#include "stdlib.h"

namespace std {

inline void abort() { return ::abort(); }

}
#endif


