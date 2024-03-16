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
// DEBT: Checking for AVR here is quite crude
#if _ISOC99_SOURCE || __AVR__
typedef ::int_least16_t int_least16_t;
typedef ::int_least32_t int_least32_t;
typedef ::int_least64_t int_least64_t;

typedef ::uint_least16_t uint_least16_t;
typedef ::uint_least32_t uint_least32_t;
typedef ::uint_least64_t uint_least64_t;

typedef ::int_fast16_t  int_fast16_t;
typedef ::int_fast32_t  int_fast32_t;
typedef ::int_fast64_t  int_fast64_t;
#else
typedef ::uint16_t int_least16_t;
typedef ::uint32_t int_least32_t;
typedef ::uint64_t uint_least64_t;
#endif

}
#endif

// FIX: Thinking to reverse this, make estd ones the "leading" ones since Dragonbox
// author doesn't himself lean towards aliasing out std to this degree

// Consumers SHOULD NOT use these but rather the above aliased std:: versions.  This exists in
// support of edge case libraries that want a full alias to std, such as dragonbox
namespace estd {

typedef std::int_least16_t int_least16_t;
typedef std::int_least32_t int_least32_t;
typedef std::int_least64_t int_least64_t;
typedef std::uint_least32_t uint_least32_t;
typedef std::uint_least64_t uint_least64_t;

}
