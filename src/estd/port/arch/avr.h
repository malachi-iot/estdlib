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

inline void* operator new(unsigned, void* p) {return p; }
inline void* operator new[](unsigned, void *p) { return p; }