#pragma once

#include "platform.h"
#include <stdint.h>
#include <stdlib.h>

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

// DEBT: Time to rework this whole maxStringLength arrangement.
// 1.  Its silent failures cause problems.  Unsupported types should generate compile time
//     errors, not a '0'
// 2.  The whole specialization on int32 etc not working  for some CPUs is confusing and crusty.  Get to
//     the bottom of that

// EXCLUDES null termination but room for a - sign
// a value of 0 indicates type not supported
template<class T>
ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength() { return 0; }

// DEBT: Unsure why exactly on some platforms int/int32_t seem to overlap and others
// don't.  Pertains to ESTD_ARCH_BITNESS checks below


template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<char>() { return 1; }
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<uint8_t>() { return 3; }
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<int8_t>() { return 4; }
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<uint16_t>() { return 5; }
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<int16_t>() { return 6; }

// On 32-bit Raspbian we need to exclude this
#if !(ESTD_MCU_ARM && ESTD_ARCH_BITNESS == 32 && INT32_MAX == INT_MAX)
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<uint32_t>() { return 10; }
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<int32_t>() { return 11;}
#endif

template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<uint64_t>() { return 21;}
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<int64_t>() { return 20;}
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<float>() { return 32; }
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<double>() { return 64; }

#if ESTD_MCU_ARM && ESTD_ARCH_BITNESS == 32 && INT32_MAX == INT_MAX
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<int>() { return 11; }
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<unsigned>() { return 11; }
#endif

// https://github.com/brucehoult/riscv-meta/blob/master/doc/src/rv128.md
// https://riscv.org/wp-content/uploads/2015/01/riscv-calling.pdf
#if __riscv
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<int>() { return 11; }
template<> ESTD_CPP_CONSTEXPR_RET uint8_t maxStringLength<unsigned>() { return 10; }
#endif


extern const char VALIDATE_NULLSTR_ERROR[];
extern const char VALIDATE_STRTOOLONG_ERROR[];
extern const char VALIDATE_FORMAT_ERROR[];

extern const char TYPENAME_INT[];
extern const char TYPENAME_CHARPTR[];

template<class T> PGM_P validateString(const char* input);
template<class T, class TChar> char* toString(TChar* output, T input);

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
