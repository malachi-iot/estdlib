#pragma once

#include "exception.h"


// Spec calls for variant assignments to demand constructible AND assignable from
// all alternatives.  When this flag is 0, that requirement is enforced.  Default
// is to 1, which permits a mix of either constructible OR assignable alternatives
// (naming this 'permissive' since it marks a deviation from spec, rather than 'strict'
// which would imply stricter than spec)
// NOTE: Applies to variant buddies: variant_storage, variant, optional, expected
#ifndef FEATURE_ESTD_VARIANT_PERMISSIVE_ASSIGNMENT
#define FEATURE_ESTD_VARIANT_PERMISSIVE_ASSIGNMENT 1
#endif


// Spec is inspecific whether operations which gracefully empty a variant of its
// alternative immediately run the alternative destructor or whether it waits until
// a later go-out-of-scope or reassignment.  Spec seems to imply it's the latter.
// We default to the former (eager)
#ifndef FEATURE_ESTD_VARIANT_EAGER_DESTRUCT
#define FEATURE_ESTD_VARIANT_EAGER_DESTRUCT 1
#endif


// We trust SFINAE to choose the right assignment helpers in variant_storage.  However, bugs
// can happen.  This flag (defaulting to on) generates extra runtime code to alert/abort if
// we encounter a condition where a non convertible (permissive convert) is actually attempted.
// Appears to be mainly an artifact of -fpermissive or similar, where things like const char* -> int
// are considered during SFINAE and then complained about
// Turn this off to optimize code more, since it ideally isn't needed
#ifndef FEATURE_ESTD_VARIANT_STRICT_CONVERSION
#define FEATURE_ESTD_VARIANT_STRICT_CONVERSION 1
#endif

