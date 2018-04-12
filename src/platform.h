#pragma once

#if __cplusplus >= 201103L
#define FEATURE_ESTDLIB_MOVESEMANTIC
#define CONSTEXPR constexpr
#define NULLPTR nullptr
#else
#define CONSTEXPR const
#define NULLPTR NULL
#endif

