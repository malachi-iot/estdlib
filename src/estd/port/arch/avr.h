/**
 * Under platformio AVR, placement new operators are not declared or defined - and <new> is MIA
 * Maybe other AVR compilers have this and pio's GCC is too old.
 * 
 * @file avr.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

// Used to have new operators, but now we've put them out into new.h

// Space is so limited on AVR we default this to on.
#define FEATURE_ESTD_AGGRESSIVE_BITFIELD 1

// https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
#if __AVR_ARCH__
#define ESTD_MCU_AVR 1
#define ESTD_MCU_ATMEL_AVR 1
#endif

// DEBT: Someone correct me if I'm wrong, but AVR GCC compiler lies and says
// initializer lists are present when they aren't
#undef __cpp_initializer_lists