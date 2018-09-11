#pragma once

#include "platform.h"
#include <stdint.h>
#include <stdlib.h>

#ifdef ESTD_ARDUINO
#include <Arduino.h>
#endif

namespace estd {

namespace experimental {

template <class TChar, class T>
struct string_convert_traits;

}

namespace internal {

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

// EXCLUDING null termination but room for a - sign
// a value of 0 indicates type not supported
template<class T> CONSTEXPR uint8_t maxStringLength();

template<> inline CONSTEXPR uint8_t maxStringLength<char>() { return 1; }
template<> inline CONSTEXPR uint8_t maxStringLength<uint8_t>() { return 3; }
template<> inline CONSTEXPR uint8_t maxStringLength<int8_t>() { return 4; }
template<> inline CONSTEXPR uint8_t maxStringLength<uint16_t>() { return 5; }
template<> inline CONSTEXPR uint8_t maxStringLength<int16_t>() { return 6; }
template<> inline CONSTEXPR uint8_t maxStringLength<uint32_t>() { return 10; }
template<> inline CONSTEXPR uint8_t maxStringLength<int32_t>() { return 11;}
template<> inline CONSTEXPR uint8_t maxStringLength<uint64_t>() { return 21;}
template<> inline CONSTEXPR uint8_t maxStringLength<int64_t>() { return 20;}
template<> inline CONSTEXPR uint8_t maxStringLength<float>() { return 32; }
template<> inline CONSTEXPR uint8_t maxStringLength<double>() { return 64; }

// not supported, we can't tell how long these are
template<> inline CONSTEXPR uint8_t maxStringLength<char*>() { return 0; }
template<> inline CONSTEXPR uint8_t maxStringLength<const char*>() { return 0; }


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
template<class T, class TChar> char* toString(TChar* output, T input);

// TODO: route this to actual snprintfs etc.
template<class T, class TChar> char* toString(TChar* output, T input, size_t)
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

template<> inline PGM_P validateString<const char*>(const char*)
{
    DO_VALIDATE_FORNULL;
    return NULLPTR;
}

template<> inline PGM_P validateString<unsigned char>(const char* input)
{
    return validateString<int>(input);
}

}}
