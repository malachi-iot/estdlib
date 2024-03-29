#pragma once

// TODO: There's a dynamic array feature flag floating around in the old internal/opts.h, so liberate that

// For extremely constrained environments, one can save a few bytes by disabling the baked in
// bounds checking to things like 'append'.  As always, if you elect to skip bounds checking,
// take extra care!
// NOTE: Disabled since feature is not 100% working.  Specifically, an ambiguity exists in internal::dynamic_array
// as to whether abort/except should happen there or not
#ifndef FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
#define FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK 0
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

// std::vector demands a proper assignment operator be present to satisfy
// 'CopyAssignable' and 'MoveAssignable' during an insert.  We are more
// relaxed and can fall back to a copy or move constructor, respectively.
// Set this flag to '1' if the more strict std behavior is desired.
#ifndef FEATURE_ESTD_DYNAMIC_ARRAY_STRICT_ASSIGNMENT
#define FEATURE_ESTD_DYNAMIC_ARRAY_STRICT_ASSIGNMENT 0
#endif

// WHen doing inserts or deletes, if trivial type is detected,
// use memmove or memcpy instead of regular estd::copy and friends
// this is EXPERIMENTAL and not fully functional at this time
#ifndef FEATURE_ESTD_DYNAMIC_ARRAY_MEMMOVE
#define FEATURE_ESTD_DYNAMIC_ARRAY_MEMMOVE 0
#endif
