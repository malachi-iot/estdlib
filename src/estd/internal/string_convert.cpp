//#include "fact/c_types.h"
#include "string_convert.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifndef ESTD_ARDUINO
#define PROGMEM
#endif

#if defined(USING_SPRINTF) || defined(ESTD_POSIX)
#include <inttypes.h>
#endif

// TODO: Make all these toStrings return actual number of bytes used
// TODO: Make all these toStrings take a maximum length parameter


namespace estd { namespace internal {

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



template<> char* toString(char* output, char input)
{
  output[0] = input;
  output[1] = 0;
  return output;
}


#if defined(__AVR__) or defined(__SAMD21G18A__) or \
    (defined(ESP8266) and defined(ARDUINO)) or defined(ESP32)
#define STDLIB_NONISO
#define STDLIB_NONISO_ITOA
#define STDLIB_NONISO_UTOA
#endif

#ifdef STDLIB_NONISO
#include <stdlib.h>

template<> char* toString(char* output, int input)
{
    return itoa(input, output, 10);
}

template<> char* toString(char* output, unsigned int input)
{
    return utoa(input, output, 10);
}

#ifdef ESP32
template<> char* toString(char* output, int16_t input)
{
    return itoa(input, output, 10);
}

template<> char* toString(char* output, uint16_t input)
{
    return utoa(input, output, 10);
}
#endif

#elif defined(ESP_OPEN_RTOS)
// ESP RTOS has its own flavor of stdlib calls
// ALL THESE UNTESTED, but worked well when embedded in ostream
template<> char* toString(char* output, int input)
{
  return __itoa(input, output, 10);
}


template<> char* toString(char* output, unsigned int input)
{
  return __utoa(input, output, 10);
}
#else
#include <stdio.h>

template<> char* toString(char* output, int input)
{
    sprintf(output, "%d", input);
    return output;
}

template<> char* toString(char* output, uint8_t input)
{
  sprintf(output, "%u", input);
  return output;
}


template<> char* toString(char* output, uint16_t input)
{
  sprintf(output, "%u", input);
  return output;
}


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

#if defined(__AVR__)
template<> char* toString(char* output, float input)
{
    constexpr size_t len = maxStringLength<float>();
    // UNTESTED
    dtostrf(input, len, len / 2, output);
    return output;
}
#else
#include <stdio.h>
template<> char* toString(char* output, float input)
{
  sprintf(output, "%f", input);
  return output;
}
#endif



template<> PGM_P getTypeName<const char*>()
{
  return TYPENAME_CHARPTR;
}

template<> PGM_P getTypeName<int>()
{
  return TYPENAME_INT;
}

}}
