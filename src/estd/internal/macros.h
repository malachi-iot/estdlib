// These are utility wrappers for forward and backward compatibility
// C++11-ish wrapper section
#pragma once

#include "feature/std.h"
#include "feature/cpp.h"
#include "macro/c++11_emul.h"

// NOTE: Only applies to empty constructor.  You're on your own for the "more complex" varieties
#ifdef FEATURE_CPP_DEFAULT_CTOR
#define ESTD_CPP_DEFAULT_CTOR(class_name)   constexpr class_name() = default;
#else
#define ESTD_CPP_DEFAULT_CTOR(class_name)   class_name() {}
#endif

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

// DEBT: Put these elsewhere
// NOTE: Not using feature test has_macro since I've seen that glitch
// out in llvm IIRC
#if __cplusplus >= 201703L
#define ESTD_CPP_ATTR_FALLTHROUGH   [[fallthrough]]
#define ESTD_CPP_ATTR_NODISCARD     [[nodiscard]]
#else
#define ESTD_CPP_ATTR_FALLTHROUGH
#define ESTD_CPP_ATTR_NODISCARD
#endif
