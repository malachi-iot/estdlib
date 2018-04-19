#pragma once

#if __cplusplus >= 201103L
#define FEATURE_ESTDLIB_MOVESEMANTIC
#define FEATURE_CPP_MOVESEMANTIC
#define FEATURE_CPP_ALIASTEMPLATE
#define FEATURE_CPP_STATIC_ASSERT
#define FEATURE_CPP_VARIADIC
#define FEATURE_CPP_CONSTEXPR

#define CONSTEXPR constexpr
#define NULLPTR nullptr
#else
#define CONSTEXPR const
#define NULLPTR NULL
#endif

#include_next "platform.h"
