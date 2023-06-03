#pragma once

// FIX: eventually do this as <estd/opts.h> so users can override it
#include "opts.h"
#include "../port/identify_platform.h"
#include "version.h"
#include "feature/std.h"
#include "feature/cpp.h"
#include "feature/toolchain.h"
#include "macros.h"


// TODO: Identify a better way to identify presence of C++ iostreams
#if __ADSPBLACKFIN__ || defined(__unix__) || defined(_MSC_VER) || (defined (__APPLE__) && defined (__MACH__))
#ifndef FEATURE_ESTD_IOSTREAM       // this one will eventually be our estd::ostream
#define FEATURE_ESTD_IOSTREAM_NATIVE
#endif
#endif


// Now with all the capabilities set up, do more complex platform support,
// if necessary
#include "../port/support_platform.h"
