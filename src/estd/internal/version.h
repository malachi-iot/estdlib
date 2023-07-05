// guidance from
// https://en.cppreference.com/w/cpp/header/ciso646
// https://stackoverflow.com/questions/31657499/how-to-detect-stdlib-libc-in-the-preprocessor 
//
// this is our flavor of https://en.cppreference.com/w/cpp/header/version since
// as of today C++20 isn't highly available in the embedded world
#include <iso646.h>
#include "../port/version.h"

#if __GLIBCXX__             // GNU libstdc++
#define FEATURE_STD     1
#elif _LIBCPP_VERSION       // LLVM libc++
#define FEATURE_STD     2
#elif _CPPLIB_VER           // MSVC
#define FEATURE_STD     3
#else
//#warning No identified STD C++
#endif
