#pragma once

// DEBT: Relate/combine with FEATURE_ESTD_OSTREAM_OCTAL
// from_chars treatment of octal is inherent,
// this only applies to sto_mode = true
// 11JAN26 MB DEBT: Phase this out, there is no point.  Octal availability is inherent
// with from_chars, so there's no real penalty to enabling this all the time.  Remove
// after 18JAN26.
#ifndef FEATURE_ESTD_FROM_CHARS_OCTAL
#define FEATURE_ESTD_FROM_CHARS_OCTAL 1
#endif

// When true, dogfood our own from_chars into stoi and friends.
// When false, fall back on C library version
#ifndef FEATURE_ESTD_FROM_CHARS_STOI
#define FEATURE_ESTD_FROM_CHARS_STOI 1
#endif

#ifndef FEATURE_ESTD_CHARCONV_AGGRESSIVE_INLINE
#define FEATURE_ESTD_CHARCONV_AGGRESSIVE_INLINE 1
#endif

#if FEATURE_ESTD_CHARCONV_AGGRESSIVE_INLINE
#define ESTD_CHARCONV_INLINE inline __attribute__((always_inline, optimize("tree-sra,dse")))
#define ESTD_CHARCONV_TREE_SRA __attribute__((optimize("tree-sra,dse")))
#else
#define ESTD_CHARCONV_INLINE
#define ESTD_CHARCONV_TREE_SRA
#endif
