// Raw C++ language feature determination
#pragma once

// TODO: consider utilizing
// http://en.cppreference.com/w/User:D41D8CD98F/feature_testing_macros and
// http://en.cppreference.com/w/cpp/experimental
#if __cplusplus >= 201103L
#define FEATURE_CPP_AUTO
#define FEATURE_CPP_ALIASTEMPLATE   // 'using' keyword in conjunction with templates
#define FEATURE_CPP_ALIGN
#define FEATURE_CPP_CONSTEXPR
#define FEATURE_CPP_DECLTYPE
#define FEATURE_CPP_MOVESEMANTIC
#define FEATURE_CPP_STATIC_ASSERT
#define FEATURE_CPP_VARIADIC
#define FEATURE_CPP_LAMBDA
#define FEATURE_CPP_RANGED_FORLOOP
#define FEATURE_CPP_INITIALIZER_LIST
#define FEATURE_CPP_DEFAULT_CTOR
#define FEATURE_CPP_DELETE_CTOR
#define FEATURE_CPP_DEFAULT_FUNCDEF
#define FEATURE_CPP_DEFAULT_TARGS   // default template arguments for a function template
#define FEATURE_CPP_ENUM_CLASS
#define FEATURE_CPP_INLINE_STATIC   // whether static variables can be initialized inline
#define FEATURE_CPP_CHAR16_T
#define FEATURE_CPP_NULLPTR
#define FEATURE_CPP_OVERRIDE
#define FEATURE_CPP_NOEXCEPT
#define FEATURE_CPP_USER_LITERAL    // https://en.cppreference.com/w/cpp/language/user_literal
#define FEATURE_CPP_INLINE_NAMESPACE


#if defined(__STDC_LIB_EXT1__)
// see http://en.cppreference.com/w/c/string/byte/strncpy
#define FEATURE_CPP_STRNCPY_S
#endif

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
// for inline method constexpr as described https://en.cppreference.com/w/cpp/language/constexpr
#define FEATURE_CPP_CONSTEXPR_METHOD
#define FEATURE_CPP_DEDUCTION_GUIDES
#define FEATURE_CPP_INLINE_VARIABLES    // whether freestanding/static variables can be declared inline
#endif

#elif defined(__GXX_EXPERIMENTAL_CXX0X__)
// for old c++0x mode, which was in affect for quite some time before c++11
// assumes GCC 4.3 or higher
// GCC had early support for many c++11 features
// https://gcc.gnu.org/projects/cxx-status.html

#define FEATURE_CPP_DEFAULT_TARGS

#if __cpp_variadic_templates >= 200704
#define FEATURE_CPP_VARIADIC
#endif

#if __cpp_decltype >= 200707
#define FEATURE_CPP_DECLTYPE
#endif

#if __cpp_alias_templates >= 200704
#define FEATURE_CPP_ALIASTEMPLATE
#endif

#if __cpp_constexpr >= 200704
#define FEATURE_CPP_CONSTEXPR
#endif

#if __cpp_static_assert >= 200410
#define FEATURE_CPP_STATIC_ASSERT
#endif

#if __cpp_lambdas >= 200907
#define FEATURE_CPP_LAMBDA
#endif

//#  include <features.h>
#  if __GNUC_PREREQ(4,8)
#define FEATURE_CPP_ALIGN
#  endif
#  if __GNUC_PREREQ(4,7)
#define FEATURE_CPP_OVERRIDE
#  endif
#  if __GNUC_PREREQ(4,6)
#define FEATURE_CPP_NULLPTR
#define FEATURE_CPP_MOVESEMANTIC
#  endif
#  if __GNUC_PREREQ(4,4)
#define FEATURE_CPP_ENUM_CLASS
#define FEATURE_CPP_AUTO
#define FEATURE_CPP_DEFAULT_CTOR
#define FEATURE_CPP_DELETE_CTOR
#define FEATURE_CPP_DEFAULT_FUNCDEF
#  endif
#  if __GNUC_PREREQ(4,2)
#define FEATURE_CPP_TYPEOF
#  endif
#else

#ifdef __cpp_initializer_lists
#define FEATURE_CPP_INITIALIZER_LIST
#endif
#endif

// DEBT: Consolidate this with FEATURE_CPP_PUSH_MACRO
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
#define FEATURE_PRAGMA_PUSH_MACRO
#endif

