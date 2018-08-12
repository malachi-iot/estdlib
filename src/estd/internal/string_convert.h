#pragma once

#include "platform.h"
#include <stdint.h>
#include <stdlib.h>

namespace estd { namespace internal {

template<class T> T fromString(const char* input);

template<> inline unsigned char fromString(const char* input)
{
    // TODO: use strtol and friends if available, atoi is deprecated
    return (unsigned char) atoi(input);
}

template<> inline int fromString(const char* input)
{
    // TODO: use strtol and friends if available, atoi is deprecated
    return atoi(input);
}

template<> inline float fromString(const char* input)
{
    return atof(input);
}

template<> inline unsigned long fromString(const char* input)
{
    return strtoul(input, NULL, 10);
}

template<> inline const char* fromString(const char* input)
{
    return input;
}

template<> inline char fromString(const char* input)
{
    return input[0];
}

template<> inline unsigned short fromString(const char* input)
{
    return (unsigned short) strtoul(input, NULL, 10);
}

// EXCLUDING null termination
template<class T> constexpr uint8_t maxStringLength();

template<> inline constexpr uint8_t maxStringLength<char>() { return 1; }
template<> inline constexpr uint8_t maxStringLength<uint8_t>() { return 3; }
template<> inline constexpr uint8_t maxStringLength<int8_t>() { return 4; }
template<> inline constexpr uint8_t maxStringLength<uint16_t>() { return 5; }
template<> inline constexpr uint8_t maxStringLength<int16_t>() { return 6; }
template<> inline constexpr uint8_t maxStringLength<uint32_t>() { return 10; }
template<> inline constexpr uint8_t maxStringLength<float>() { return 32; }
template<> inline constexpr uint8_t maxStringLength<double>() { return 64; }


extern const char VALIDATE_NULLSTR_ERROR[];
extern const char VALIDATE_STRTOOLONG_ERROR[];
extern const char VALIDATE_FORMAT_ERROR[];

extern const char TYPENAME_INT[];
extern const char TYPENAME_CHARPTR[];

#ifndef ARDUINO
// FIX: need a mini-noduino.h...
#undef PGM_P
#define PGM_P const char*
#endif

template<class T> PGM_P validateString(const char* input);
template<class T> char* toString(char* output, T input);

// TODO: route this to actual snprintfs etc.
template<class T> char* toString(char* output, T input, size_t maxlen)
{
    return toString(output, input);
}

template<class T> PGM_P getTypeName();

#ifdef VALIDATE_FEATURE_NULLCHECK
#define DO_VALIDATE_FORNULL { if(!input) return VALIDATE_NULLSTR_ERROR; }
#else
#define DO_VALIDATE_FORNULL
#endif

// template function prototype so that <unsigned char> inline variant works
template<> PGM_P validateString<int>(const char* input);

template<> inline PGM_P validateString<const char*>(const char* input)
{
    DO_VALIDATE_FORNULL;
    return nullptr;
}

template<> inline PGM_P validateString<unsigned char>(const char* input)
{
    return validateString<int>(input);
}

}}