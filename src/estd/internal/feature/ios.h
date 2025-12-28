#pragma once

#include "../platform.h"

#ifndef FEATURE_ESTD_IOS_GCOUNT
#define FEATURE_ESTD_IOS_GCOUNT 1
#endif

#ifndef FEATURE_ESTD_AGGRESSIVE_BITFIELD
#define FEATURE_ESTD_AGGRESSIVE_BITFIELD 1
#endif


#ifndef FEATURE_ESTD_OSTREAM_SETW
#define FEATURE_ESTD_OSTREAM_SETW 1
#define FEATURE_ESTD_OSTREAM_SETFILL 1
#define FEATURE_ESTD_OSTREAM_SETIOSALIGN 1
#endif

#ifndef FEATURE_ESTD_OSTREAM_OCTAL
#define FEATURE_ESTD_OSTREAM_OCTAL 1
#endif

// Track ostream.precision() and fixed/scientific/etc formats
#ifndef FEATURE_ESTD_OSTREAM_FLOAT
#define FEATURE_ESTD_OSTREAM_FLOAT 0
#endif

// https://en.cppreference.com/w/cpp/io/basic_ios/init.html tells us 6 for default
#ifndef ESTD_OSTREAM_DEFAULT_PRECISION
#define ESTD_OSTREAM_DEFAULT_PRECISION 6
#endif


