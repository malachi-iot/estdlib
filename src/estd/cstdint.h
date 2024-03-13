#pragma once

#include "internal/platform.h"

#ifdef FEATURE_STD_CSTDINT
#include <cstdint>
#else
#include <stdint.h>

namespace std {

typedef ::intmax_t intmax_t;
typedef ::uintmax_t uintmax_t;

// "since C99" https://en.cppreference.com/w/c/types/integer
#if _ISOC99_SOURCE
typedef ::int_least16_t int_least16_t;
typedef ::int_least32_t int_least32_t;
typedef ::int_least64_t int_least64_t;

typedef ::int_fast16_t  int_fast16_t;
typedef ::int_fast32_t  int_fast32_t;
typedef ::int_fast64_t  int_fast64_t;
#else
typedef ::uint16_t int_least16_t;
typedef ::uint32_t int_least32_t;
typedef ::uint64_t int_least64_t;
#endif

}
#endif
