#pragma once

#include "../platform.h"

// DEBT: feature flags at odds with each other's paradigm.
// First one a '0' means use std
// Second one a '1' means use std

// When 1, we favor our own implementations of heap algorithm
// When 0 (default), we favor std implementation
// If available, default to utilizing regular std heap mechanisms since we expect
// their algorithms to be more optimized than mine.
#if !defined(FEATURE_ESTD_ALGORITHM_HEAP)
// Temporarily forcing this on while we dev
#define FEATURE_ESTD_ALGORITHM_HEAP 1
// Following works fine
//#define FEATURE_ESTD_ALGORITHM_HEAP !FEATURE_STD_ALGORITHM
#endif

// Aliases out general operations to std library in anticipation of them being quicker
#ifndef FEATURE_ESTD_ALGORITHM_OPT
//#define FEATURE_ESTD_ALGORITHM_OPT FEATURE_STD_ALGORITHM
#endif
