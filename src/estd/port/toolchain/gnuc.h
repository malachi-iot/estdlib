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

// First, see if we need to explicitly include any of the stuff in
// GLIBC's (or similar, like newlib) features.h
#if !defined(__GNUC_PREREQ)

#ifdef __AVR_LIBC_VERSION__
// Won't have <features.h>, may need to manually create __GNUC_PREREQ
// though ASF seems to have it somewhere
#elif defined(__NEWLIB__)
// TODO: Newlib does have it I believe in <sys/features.h>
// Until we verify that, manually create it
// Generally, __GLIBC__ is actually defined *inside* features.h so this might
// be a partially broken test
#elif defined(__linux__) || defined(__GLIBC__)
// Only a linux/glibc thing:
// https://stackoverflow.com/questions/259248/how-to-test-the-current-version-of-gcc-at-compile-time
// GLIBC detection gleaned from
// http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/common.h
#include <features.h>
#endif

// If after all of above we still don't have one, manually create it
#if !defined(__GNUC_PREREQ)
#define __GNUC_PREREQ(major, minor) (__GNUC__ > major || (__GNUC__ == major && __GNUC__MINOR__ >= minor))
#endif

#endif


#ifndef __BYTE_ORDER__
#ifdef __ORDER_LITTLE_ENDIAN__
#error Unexpected: __BYTE_ORDER__ not present, but __ORDER_LITTLE_ENDIAN__ is
#endif
#define __ORDER_LITTLE_ENDIAN__		1
#define __ORDER_BIG_ENDIAN__		2
#define __ORDER_PDP_ENDIAN__		3
#if defined(__ADSPBLACKFIN__)
#define __BYTE_ORDER__ 				__ORDER_LITTLE_ENDIAN__
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
