#pragma once

// TODO: consider utilizing
// http://en.cppreference.com/w/User:D41D8CD98F/feature_testing_macros and
// http://en.cppreference.com/w/cpp/experimental
#if __cplusplus >= 201103L
#define FEATURE_CPP_ALIASTEMPLATE
#define FEATURE_CPP_CONSTEXPR
#define FEATURE_CPP_DECLTYPE
#define FEATURE_CPP_DECLVAL
#define FEATURE_CPP_MOVESEMANTIC
#define FEATURE_CPP_STATIC_ASSERT
#define FEATURE_CPP_VARIADIC
#define FEATURE_CPP_LAMBDA
#define FEATURE_CPP_RANGED_FORLOOP
#define FEATURE_CPP_INITIALIZER_LIST
#define FEATURE_CPP_DEFAULT_FUNCDEF

#if defined(__STDC_LIB_EXT1__)
// see http://en.cppreference.com/w/c/string/byte/strncpy
#define FEATURE_CPP_STRNCPY_S
#endif

#define OVERRIDE override
#define CONSTEXPR constexpr
#define NULLPTR nullptr

// C++17 onward features go here
#if __cplusplus >= 201703L
#endif

#else
#define OVERRIDE
#define CONSTEXPR const
#define NULLPTR NULL
#endif

// TODO: Identify a better way to identify presence of C++ iostreams
#if __ADSPBLACKFIN__ || defined(__unix__) || defined(_MSC_VER) || (defined (__APPLE__) && defined (__MACH__))
#ifndef FEATURE_ESTD_IOSTREAM       // this one will eventually be our estd::ostream
#define FEATURE_ESTD_IOSTREAM_NATIVE
#endif
#endif
