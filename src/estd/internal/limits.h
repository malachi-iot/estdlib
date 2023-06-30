#pragma once

#ifdef FEATURE_STD_CLIMITS
#include <climits>
#else
// NOTE: Keep an eye on this, make sure we are pulling in proper standard limits.h
// -- and in fact, our filename probably should be climits.h
#include <limits.h>

#endif

#ifndef CHAR_BIT
#warning CHAR_BIT not set, defaulting to 8 bits
#define CHAR_BIT 8
#endif

#if CHAR_BIT != 8
#error "Only 8 bit-wide bytes supported"
#endif

namespace estd { namespace internal {

}}
