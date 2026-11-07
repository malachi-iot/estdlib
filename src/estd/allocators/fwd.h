#pragma once

#ifndef FEATURE_ESTD_ALLOC_AGGRESSIVE_INLINE
#define FEATURE_ESTD_ALLOC_AGGRESSIVE_INLINE 1
#endif

#if defined(__GNUC__) and !defined(__clang__) and FEATURE_ESTD_ALLOC_AGGRESSIVE_INLINE
#define ESTD_ALLOC_INLINE inline __attribute__((always_inline, optimize("tree-sra,dse")))
#define ESTD_ALLOC_INLINE_FLATTEN inline __attribute__((always_inline, flatten, optimize("tree-sra,dse")))
#else
#define ESTD_ALLOC_INLINE
#define ESTD_ALLOC_INLINE_FLATTEN
#endif
