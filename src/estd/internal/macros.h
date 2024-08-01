// These are utility wrappers for forward and backward compatibility
// C++11-ish wrapper section
#pragma once

#include "feature/std.h"
#include "feature/cpp.h"
#include "macro/c++11_emul.h"
#include "macro/cpp.h"

#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
// DEBT: May want this to be FEATURE_TOOLCHAIN_PUSH_MACRO
#define FEATURE_CPP_PUSH_MACRO 1
#endif


#ifdef FEATURE_CPP_OVERRIDE
#define OVERRIDE override
#else
#define OVERRIDE
#endif

#ifdef FEATURE_CPP_NULLPTR
#define NULLPTR nullptr
#else
#define NULLPTR NULL
#endif

#ifdef FEATURE_CPP_CONSTEXPR
#define CONSTEXPR constexpr
#define EXPLICIT explicit
#else
#define CONSTEXPR const
#define EXPLICIT
#endif

#ifdef __cpp_conditional_explicit
#define CONSTEXPR_EXPLICIT(conditional) constexpr explicit(conditional)
#elif __cpp_constexpr
#define CONSTEXPR_EXPLICIT(conditional) constexpr explicit
#else
#define CONSTEXPR_EXPLICIT(conditional) inline
#endif

#ifdef FEATURE_CPP_NOEXCEPT
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

