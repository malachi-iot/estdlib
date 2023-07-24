#pragma once

// TODO: There's a dynamic array feature flag floating around in the old internal/opts.h, so liberate that

// For extremely constrained environments, one can save a few bytes by disabling the baked in
// bounds checking to things like 'append'.  As always, if you elect to skip bounds checking,
// take extra care!
#ifndef FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
#define FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK 1
#endif

// By default, append will add everything it can and truncate anything that goes over the
// limit.  This conflicts with "strong exception safety guarantee"
// https://en.cppreference.com/w/cpp/string/basic_string/append - so we default that feature
// to off when exceptions are present
#ifndef FEATURE_ESTD_DYNAMIC_ARRAY_APPEND_TRUNC
#if __cpp_exceptions
#define FEATURE_ESTD_DYNAMIC_ARRAY_APPEND_TRUNC 0
#else
#define FEATURE_ESTD_DYNAMIC_ARRAY_APPEND_TRUNC 1
#endif
#endif