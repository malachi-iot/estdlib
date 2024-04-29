#include <estd/ostream.h>

#include <estd/string.h>

namespace avr {

void loop1(estd::layer2::string<> name);
void loop2();
void test_array();

}

#define USE_IOS 1

extern estd::arduino_ostream cout;

#if defined(__AVR__)
#if !defined(EXP_PGM_STRING)
#define EXP_PGM_STRING 1
#endif
#if !defined(EXP_PGM_ARRAY)
#define EXP_PGM_ARRAY 1
#endif
#endif
