#pragma once

// For use with FEATURE_ESTD_COMPILE_STRICTNESS, but available for other uses
// These numbers are subject to change!
#define ESTD_LEVEL_NONE     0
#define ESTD_LEVEL_LOW      1
#define ESTD_LEVEL_MEDIUM   2
#define ESTD_LEVEL_HIGH     3
#define ESTD_LEVEL_MAX      4

// How chatty we should be about #warnings, unexpected conditions and general clues
// 0 = silent - no warnings emitted
// 1 = highly pertinent information only, indications of probable misbehavior
// 2 = indications of potential, but not necessarily probable, misbehavior
// 3 = very verbose warnings & pragma messages giving many possible clues
#ifndef FEATURE_ESTD_COMPILE_VERBOSITY
#define FEATURE_ESTD_COMPILE_VERBOSITY 1
#endif


// Determines how many assumptions we're willing to let slide vs strictly enforcing
// correctness.  Varies on a case by case basis
// NOTE: Raising this higher will result in more asserts, so watch out for that
// 0 = none/completely lax
// 1 = low
// 2 = medium
// 3 = high
// 4 = max
// Not designed to be used standalone but rather as a feeder for other STRICT settings
// so that they can be overridden piecemeal.
// It is planned that experimental features are disabled at strictness level 3 and higher
#ifndef FEATURE_ESTD_COMPILE_STRICTNESS
#define FEATURE_ESTD_COMPILE_STRICTNESS 1
#endif

// Are experimental features enabled in an overall sense
#define FEATURE_ESTD_EXPERIMENTAL   (FEATURE_ESTD_COMPILE_STRICTNESS < ESTD_LEVEL_HIGH)
