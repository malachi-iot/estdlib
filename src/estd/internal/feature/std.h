#pragma once

#include "../version.h"

// DEBT: More than likely this is an artifact of an old G++, not AVR/SAMD specific
#if !defined(__AVR__) //&& !defined(__SAMD21G18A__)
#define FEATURE_STD_NEW
#endif

// some platforms have c++98 onward but don't have STL headers.  Explicitly state when we *do*
// have them
#if !defined(ARDUINO) || \
    FEATURE_STD

#define FEATURE_STD_ALGORITHM
#define FEATURE_STD_CSTDDEF

#if __ADSPBLACKFIN__
#define FEATURE_STD_CLIMITS
#endif

#if __cplusplus >= 201103L
// https://stackoverflow.com/questions/13642827/cstdint-vs-stdint-h indicates it's a c++11 feature, as
// corroborated by https://en.cppreference.com/w/cpp/types/integer.  stackoverflow also indicates it's
// not present in VC2008, but hopefully now years later it is in VC
#define FEATURE_STD_CSTDINT
#define FEATURE_STD_CASSERT
#define FEATURE_STD_INITIALIZER_LIST
#define FEATURE_STD_THREAD
#endif  // end C++11

#define FEATURE_STD_ITERATOR
#define FEATURE_STD_MEMORY
#define FEATURE_STD_STRING
#define FEATURE_STD_TYPE_TRAITS
#define FEATURE_STD_UTILITY

#endif

#if __STDC_VERSION__ >= 199901L || __cplusplus >= 201103L
#define FEATURE_STD_INTTYPES
#endif
