#pragma once

#include "../platform.h"

// When 1, we favor our own implementations of heap algorithm
// When 0 (default), we favor std implementation
// If available, default to utilizing regular std heap mechanisms since we expect
// their algorithms to be more optimized than mine.
#if !defined(FEATURE_ESTD_ALGORITHM_HEAP)
#if FEATURE_STD_ALGORITHM
#define FEATURE_ESTD_ALGORITHM_HEAP 0
#else
#define FEATURE_ESTD_ALGORITHM_HEAP 1
#endif
#endif