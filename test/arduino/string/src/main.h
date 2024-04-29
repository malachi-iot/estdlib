#include <estd/ostream.h>

#include <estd/string.h>

namespace avr {

void loop1(estd::layer2::string<> name);
void loop2();
void test_array();

}

extern estd::arduino_ostream cout;

#if defined(__AVR__) && !defined(EXP_PGM_STRING)
#define EXP_PGM_STRING 1
#endif
