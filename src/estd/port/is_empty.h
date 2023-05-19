#pragma once

// __has_extension is not available in GCC
#ifdef __llvm__
// http://releases.llvm.org/3.5.1/tools/clang/docs/LanguageExtensions.html
#if __has_extension(is_empty)
#include "gcc/is_empty.h"
#endif
#elif defined(__GNUC__)
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __GNUC_PREREQ(4,3)
// https://stackoverflow.com/questions/35531309/how-is-stdis-emptyt-implemented-in-vs2015-or-any-compiler
// https://gcc.gnu.org/onlinedocs/gcc-4.5.4/gcc/Type-Traits.html
// https://www.boost.org/doc/libs/1_65_1/boost/type_traits/intrinsics.hpp
// /usr/include/c++/4.8/type_traits:516
#include "gcc/is_empty.h"
#endif  // GCC exp/4.3
#elif _MSC_VER
#include "gcc/is_empty.h"
#elif 0 // LLVM
#endif
