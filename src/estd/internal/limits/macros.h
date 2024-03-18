#pragma once

#include "../platform.h"

#ifdef FEATURE_STD_CLIMITS
#include <climits>
#include <cfloat>
#else
// NOTE: Keep an eye on this, make sure we are pulling in proper standard limits.h
// -- and in fact, our filename probably should be climits.h
#include <limits.h>
#include <float.h>

#endif

#ifndef CHAR_BIT
#if FEATURE_ESTD_COMPILE_VERBOSITY > 1
#warning CHAR_BIT not set, defaulting to 8 bits
#endif
#define CHAR_BIT 8
#endif

#if CHAR_BIT != 8
#error "Only 8 bit-wide bytes supported"
#endif


// DEBT: Would likely be better off doing SIZEOF_INTEGER and friends with template
// metaprogramming to avoid potential name collisions
// size in 8-bit bytes
#define SIZEOF_INTEGER(max) \
    (max == INT64_MAX ? 8 : \
    (max == INT32_MAX ? 4 : \
    (max == INT16_MAX ? 2 : 1)))

// TODO: Error if INT64_MAX isn't available

#if !defined(LLONG_WIDTH)

#if __SIZEOF_LONG_LONG__
#define LLONG_WIDTH (__SIZEOF_LONG_LONG__ * 8)

#elif defined(LLONG_MAX)
#define LLONG_WIDTH (SIZEOF_INTEGER(LLONG_MAX) * 8)

#elif ESTD_ARCH_BITNESS == 32
#if FEATURE_ESTD_COMPILE_STRICTNESS > 2
#error Not making assumption about 'long long' for unknown architecture
#elif FEATURE_ESTD_COMPILE_VERBOSITY > 1
#warning Assuming long long is 64 bit
#endif
#define LLONG_WIDTH     64

#elif ESTD_ARCH_BITNESS != 64 && FEATURE_ESTD_COMPILE_VERBOSITY > 0
#warning Not making assumption about 'long long' for unknown architecture
#endif
#endif

#ifndef __SIZEOF_LONG_LONG__
#define __SIZEOF_LONG_LONG__    (LLONG_WIDTH / 8)
#endif
#ifndef __SIZEOF_LONG__
#define __SIZEOF_LONG__         SIZEOF_INTEGER(LONG_MAX)
#endif
#ifndef __SIZEOF_INT__
#define __SIZEOF_INT__          SIZEOF_INTEGER(INT_MAX)
#endif
#ifndef __SIZEOF_SHORT__
#define __SIZEOF_SHORT__        SIZEOF_INTEGER(SHRT_MAX)
#endif


// As per https://en.cppreference.com/w/c/types/limits and
// https://pubs.opengroup.org/onlinepubs/009695399/basedefs/limits.h.html
#ifndef CHAR_WIDTH
#define CHAR_WIDTH      CHAR_BIT
#endif
#ifndef SHRT_WIDTH
#define SHRT_WIDTH      (__SIZEOF_SHORT__ * 8)
#endif
#ifndef INT_WIDTH
#define INT_WIDTH       (__SIZEOF_INT__ * 8)
#endif
#ifndef LONG_WIDTH
#define LONG_WIDTH      (__SIZEOF_LONG__ * 8)
#endif

// Long long already handled up above

