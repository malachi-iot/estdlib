/**
 * @file
 * clang pretends to be gnuc also, so watch out for that
 */
#pragma once

// lifted from https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Function-Names.html
#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif


#if defined(__linux__) || defined(__GLIBC__)
// Only a linux/glibc thing:
// https://stackoverflow.com/questions/259248/how-to-test-the-current-version-of-gcc-at-compile-time
// GLIBC detection gleaned from
// http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/common.h
#include <features.h>
#else
#ifndef __GNUC_PREREQ
#define __GNUC_PREREQ(major, minor) (__GNUC__ > major || (__GNUC__ == major && __GNUC__MINOR__ >= minor))
#endif
#endif


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ESTD_ARCH_LITTLE_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define ESTD_ARCH_BIG_ENDIAN
#else
#define ESTD_ARCH_PDP_ENDIAN
#endif

// Adapted from https://stackoverflow.com/questions/1505582/determining-32-vs-64-bit-in-c
// more reference https://sourceforge.net/p/predef/wiki/Architectures/
#if __x86_64__ || __ppc64__ || __aarch64__
#define ESTD_ARCH_BITNESS   64
#elif defined(__AVR__)
#define ESTD_ARCH_BITNESS   16
#else
#if FEATURE_ESTD_STRICT
#warning "Assuming 32 bit architecture"
#endif
#define ESTD_ARCH_BITNESS   32
#endif