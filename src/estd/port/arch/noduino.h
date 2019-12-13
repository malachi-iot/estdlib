/**
 * @file
 * For limited scenarios where we want Arduino specific compatibility, even when
 * we definitely aren't Arduino
 */
#pragma once

#ifndef ARDUINO
#undef PGM_P
#define PGM_P const char*
#define PROGMEM
#endif
