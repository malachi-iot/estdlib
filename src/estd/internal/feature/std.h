#pragma once

#include "../version.h"

// DEBT: More than likely this is an artifact of an old G++, not AVR/SAMD specific
#if !defined(__AVR__) //&& !defined(__SAMD21G18A__)
#define FEATURE_STD_NEW 1
#endif

// some platforms have c++98 onward but don't have STL headers.  Explicitly state when we *do*
// have them
#if !defined(ARDUINO) || \
    FEATURE_STD

#define FEATURE_STD_ALGORITHM 1
#define FEATURE_STD_CSTDDEF 1
#define FEATURE_STD_CSTDLIB 1
#define FEATURE_STD_CERRNO 1

#if __ADSPBLACKFIN__
#define FEATURE_STD_CLIMITS 1
#endif

#if __cplusplus >= 201103L
// https://stackoverflow.com/questions/13642827/cstdint-vs-stdint-h indicates it's a c++11 feature, as
// corroborated by https://en.cppreference.com/w/cpp/types/integer.  stackoverflow also indicates it's
// not present in VC2008, but hopefully now years later it is in VC
#define FEATURE_STD_CSTDINT 1
#define FEATURE_STD_CASSERT 1
#define FEATURE_STD_CHRONO 1
#define FEATURE_STD_INITIALIZER_LIST 1
#define FEATURE_STD_RATIO 1
#define FEATURE_STD_THREAD 1
#define FEATURE_STD_TYPE_TRAITS 1
#endif  // end C++11

#define FEATURE_STD_ITERATOR 1
#define FEATURE_STD_MEMORY 1
#define FEATURE_STD_STRING 1
#define FEATURE_STD_UTILITY 1

#endif

#if __STDC_VERSION__ >= 199901L || __cplusplus >= 201103L
#define FEATURE_STD_INTTYPES 1
#endif
