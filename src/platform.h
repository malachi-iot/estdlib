#pragma once

#if __cplusplus >= 201103L
#define FEATURE_ESTDLIB_MOVESEMANTIC
#define CONSTEXPR constexpr
#else
#define CONSTEXPR const
#endif

