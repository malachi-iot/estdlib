#pragma once

// TODO: There's a dynamic array feature flag floating around in the old internal/opts.h, so liberate that

// For extremely constrained environments, one can save a few bytes by disabling the baked in
// bounds checking to things like 'append'.  As always, if you elect to skip bounds checking,
// take extra care!
#ifndef FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK
#define FEATURE_ESTD_DYNAMIC_ARRAY_BOUNDS_CHECK 1
#endif