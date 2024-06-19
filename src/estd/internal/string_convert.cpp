#include "string_convert.h"
#include "../charconv.h"
#include "../type_traits.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// DEBT: Move validation mechanism out of estd completely

namespace estd { namespace internal {


#if FEATURE_ESTD_CBASE_ARRAY && __AVR__
constexpr estd::v0::pgm_array<char, 16> cbase_set<char>::lset;
#endif

#if FEATURE_ESTD_STRING_LEGACY_VALIDATION

const char VALIDATE_NULLSTR_ERROR[] PROGMEM = "Null String";
const char VALIDATE_STRTOOLONG_ERROR[] PROGMEM = "String too long";
const char VALIDATE_FORMAT_ERROR[] PROGMEM = "Invalid format";

const char TYPENAME_INT[] PROGMEM = "Integer";
const char TYPENAME_CHARPTR[] PROGMEM = "String";


// TODO: combine validate/convert functions since they happen lock step, this way
//   we can avoid double-conversion at times

template<> PGM_P validateString<char>(const char* input)
{
  DO_VALIDATE_FORNULL;
  if(strlen(input) > 1) return VALIDATE_STRTOOLONG_ERROR;

    return NULLPTR;
}


template<> PGM_P validateString<int>(const char* input)
{
  DO_VALIDATE_FORNULL;
  do
  {
    if(!isdigit(*input)) return VALIDATE_FORMAT_ERROR;
  } while(*++input);

    return NULLPTR;
}


#ifndef __AVR__
template<> PGM_P validateString<float>(const char* input)
{
  DO_VALIDATE_FORNULL;
  char* str_end;
  strtof(input, &str_end);

  return input == str_end ? VALIDATE_FORMAT_ERROR : NULLPTR;
}
#endif


template<> PGM_P validateString<double>(const char* input)
{
  DO_VALIDATE_FORNULL;
  char* str_end;
  strtod(input, &str_end);

  return input == str_end ? VALIDATE_FORMAT_ERROR : NULLPTR;
}


template <> PGM_P validateString<unsigned short>(const char* input)
{
    DO_VALIDATE_FORNULL;
    char* str_end;

    strtoul(input, &str_end, 10);

    return input == str_end ? VALIDATE_FORMAT_ERROR : NULLPTR;
}

template<> PGM_P getTypeName<const char*>()
{
  return TYPENAME_CHARPTR;
}

template<> PGM_P getTypeName<int>()
{
  return TYPENAME_INT;
}


#endif

// Phasing this out, just keeping around in case some random code needs it
// to_chars / num_put is vastly superior
#if FEATURE_ESTD_STRING_LEGACY_TOSTRING

template<> char* toString(char* output, char input)
{
  output[0] = input;
  output[1] = 0;
  return output;
}


#if defined(__AVR__) || defined(__SAMD21G18A__) || \
    (defined(ESP8266) && defined(ARDUINO)) || \
    (defined(ESP32) || defined(ESP_IDF) || defined(IDF_VER)) || \
    defined(__ARM_EABI__)
#define STDLIB_NONISO
#define STDLIB_NONISO_ITOA
#define STDLIB_NONISO_UTOA
#endif

#ifdef STDLIB_NONISO

#elif defined(ESP_OPEN_RTOS)

#else

#include <stdio.h>

// PRIu32 lives in inttypes.h, only available in C99 onward
#if __STDC_VERSION__ >= 199901L
template<> char* toString(char* output, uint32_t input)
{
    sprintf(output, "%" PRIu32, input);
    return output;
}

/*
template<> char* toString(char* output, int32_t input)
{
    sprintf(output, "%" PRIi32, input);
} */
#endif

#endif

#if defined(__AVR__)
template<> char* toString(char* output, float input)
{
    constexpr size_t len = legacy::maxStringLength<float>();
    // UNTESTED
    dtostrf(input, len, len / 2, output);
    return output;
}
#elif defined(ESTD_STM)
// TODO: stm32 *sometimes* has float-to-string conversion ability.  Use that if 
// we can detect it's present
#else
#include <stdio.h>
template<> char* toString(char* output, float input)
{
  sprintf(output, "%f", input);
  return output;
}
#endif

#endif

}}

#if FEATURE_ESTD_STRING_LEGACY_FROM_CHARS

namespace estd {
namespace legacy {

from_chars_result from_chars(const char* first, const char* last,
                             long& value, int base)
{
    char** str_end = const_cast<char**>(&last);
    errno = 0;  // seems clang demands this
    // DEBT: strtol permits + and 0x prefixes, from_chars is not
    // supposed to
    // DEBT: If we do this manually, will go faster
    value = 
    	//std::
    	strtol(first, str_end, base);
#if __cplusplus >= 201103L
    if(first == last)
        return from_chars_result { first, errc::invalid_argument };
    else
        return from_chars_result { last, errc{ errno } };
#else
    if(first == last)
        return from_chars_result(first, errc::invalid_argument);
    else
        return from_chars_result(last, errc(errno));
#endif
}

}}

#endif