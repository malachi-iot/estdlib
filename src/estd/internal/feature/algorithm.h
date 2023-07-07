#pragma once

#include "../platform.h"

// When 1, we favor our own implementations of heap algorithm
// When 0 (default), we favor std implementation
// If available, default to utilizing regular std heap mechanisms since we expect
// their algorithms to be more optimized than mine.
#if !defined(FEATURE_ESTD_ALGORITHM_HEAP)
#define FEATURE_ESTD_ALGORITHM_HEAP !FEATURE_STD_ALGORITHM
#endif