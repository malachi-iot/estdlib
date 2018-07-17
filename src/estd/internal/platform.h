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
#define FEATURE_CPP_DEFAULT_TARGS   // default template arguments for a function template
#define FEATURE_CPP_ENUM_CLASS

#if defined(__STDC_LIB_EXT1__)
// see http://en.cppreference.com/w/c/string/byte/strncpy
#define FEATURE_CPP_STRNCPY_S
#endif

#define OVERRIDE override
#define CONSTEXPR constexpr
#define NULLPTR nullptr

// C++14 onward features go here
#if __cplusplus >= 201402L
#define FEATURE_CPP_DEDUCE_RETURN
#define FEATURE_CPP_GENERIC_LAMBDA
// NOTE: This feature seems to exist unofficially in a lot of of pre c++14 implementations
// https://en.cppreference.com/w/cpp/language/integer_literal
#define FEATURE_CPP_BINARY_LITERAL
#endif

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

// some platforms have c++11 but don't have STL headers.  Explicitly state when we *do*
// have them
#if !defined(ARDUINO) && __cplusplus >= 201103L
#define FEATURE_STD_ALGORITHM
#define FEATURE_STD_CSTDDEF
#define FEATURE_STD_CASSERT
#define FEATURE_STD_INITIALIZER_LIST
#define FEATURE_STD_ITERATOR
#define FEATURE_STD_TYPE_TRAITS
#define FEATURE_STD_UTILITY
#endif
