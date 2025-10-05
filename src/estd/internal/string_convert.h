#pragma once

#include "platform.h"
#include <stdint.h>
#include <stdlib.h>

#include "../limits.h"
#include "string/to_string.h"

#ifdef ESTD_ARDUINO
#include <Arduino.h>
#else
#include "../port/arch/noduino.h"
#endif

#ifdef FEATURE_STD_INTTYPES
// Introduced with C99
// https://en.wikipedia.org/wiki/C_data_types#inttypes.h
#include <inttypes.h>
#endif

// DEBT: Ancient code, needing clean.  Don't want to wipe it out, still interesting
// move it to embr

namespace estd {

namespace internal {

extern const char VALIDATE_NULLSTR_ERROR[];
extern const char VALIDATE_STRTOOLONG_ERROR[];
extern const char VALIDATE_FORMAT_ERROR[];

extern const char TYPENAME_INT[];
extern const char TYPENAME_CHARPTR[];

template<class T> PGM_P validateString(const char* input);

template<class T> PGM_P getTypeName();

#ifdef VALIDATE_FEATURE_NULLCHECK
#define DO_VALIDATE_FORNULL { if(!input) return VALIDATE_NULLSTR_ERROR; }
#else
#define DO_VALIDATE_FORNULL
#endif

// template function prototype so that <unsigned char> inline variant works
template<> PGM_P validateString<int>(const char* input);

template<> inline PGM_P validateString<const char*>(const char*)
{
    DO_VALIDATE_FORNULL;
    return NULLPTR;
}

template<> inline PGM_P validateString<unsigned char>(const char* input)
{
    return validateString<int>(input);
}

// DEBT: Crudely removing these if not really arduino.  Instead a gcc macro push
// ought to be used
#ifndef ESTD_ARDUINO
#undef PGM_P
#undef PROGMEM
#endif

}}
