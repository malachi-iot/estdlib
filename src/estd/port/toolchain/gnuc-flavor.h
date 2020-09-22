/**
 * @file gnuc-flavor.h
 * @brief where we emulate GCC behaviors in non-GCC or non-comforming-GCC toolchains
 */
#pragma once

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


