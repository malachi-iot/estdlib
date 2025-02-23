// See README.md in this folder

#pragma once

#include "c++/ctor.h"
#include "c++/const.h"

#if __cpp_constexpr
#define ESTD_CPP_CONSTEXPR_RET constexpr
#else
#define ESTD_CPP_CONSTEXPR_RET inline
#endif

#if __cpp_ref_qualifiers
#define ESTD_CPP_REFQ &
#else
#define ESTD_CPP_REFQ
#endif

/// Wrapper for c++20 concepts for use in template lists
#if __cpp_concepts
#define ESTD_CPP_CONCEPT(T)     T
#else
#define ESTD_CPP_CONCEPT(T)     class
#endif
