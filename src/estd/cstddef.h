#pragma once

#include "internal/platform.h"

namespace estd {

#ifdef FEATURE_CPP_ENUM_CLASS
enum class byte : unsigned char {};
#else
typedef unsigned char byte;
#endif

}

#ifdef FEATURE_STD_CSTDDEF
#include <cstddef>
#else
#include <stddef.h>
namespace std {

typedef ::size_t size_t;
typedef ::ptrdiff_t ptrdiff_t;

}
#endif
