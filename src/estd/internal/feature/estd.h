#pragma once

// How chatty we should be about #warnings, unexpected conditions and general clues
// 0 = silent - no warnings emitted
// 1 = highly pertinent information only, indications of probable misbehavior
// 2 = indications of potential, but not necessarily probable, misbehavior
// 3 = very verbose warnings & pragma messages giving many possible clues
#ifndef FEATURE_ESTD_COMPILE_VERBOSITY
#define FEATURE_ESTD_COMPILE_VERBOSITY 1
#endif

// strictness level (not used yet)
// Determines how many assumptions we're willing to let slide vs strictly enforcing
// correctness.  Varies on a case by case basis
// NOTE: Raising this higher will result in more asserts, so watch out for that
// 0 = none/completely lax
// 1 = minimal
// 2 = moderate
// 3 = very
// Not designed to be used standalone but rather as a feeder for other STRICT settings
// so that they can be overridden piecemeal
#ifndef FEATURE_ESTD_COMPILE_STRICTNESS
#define FEATURE_ESTD_COMPILE_STRICTNESS 1
#endif
