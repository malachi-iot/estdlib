#pragma once

// FIX: eventually do this as <estd/opts.h> so users can override it
#include "opts.h"
#include "../port/identify_platform.h"
#include "version.h"
#include "feature/std.h"
#include "feature/cpp.h"
#include "feature/toolchain.h"
#include "macros.h"


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

// TODO: Identify a better way to identify presence of C++ iostreams
#if __ADSPBLACKFIN__ || defined(__unix__) || defined(_MSC_VER) || (defined (__APPLE__) && defined (__MACH__))
#ifndef FEATURE_ESTD_IOSTREAM       // this one will eventually be our estd::ostream
#define FEATURE_ESTD_IOSTREAM_NATIVE
#endif
#endif


// Now with all the capabilities set up, do more complex platform support,
// if necessary
#include "../port/support_platform.h"
