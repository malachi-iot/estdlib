#pragma once

#include "internal/platform.h"

#ifdef FEATURE_STD_CSTDDEF
#include <cstddef>
#else
#include <stddef.h>
namespace std {

typedef ::size_t size_t;
typedef ::ptrdiff_t ptrdiff_t;

}
#endif