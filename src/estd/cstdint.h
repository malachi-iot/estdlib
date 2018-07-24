#pragma once

#include "internal/platform.h"

#ifdef FEATURE_STD_CSTDINT
#include <cstdint>
#else
#include <stdint.h>

namespace std {

typedef ::intmax_t intmax_t;
typedef ::ptrdiff_t ptrdiff_t;

}
#endif
