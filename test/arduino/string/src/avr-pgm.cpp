#include "main.h"

// EXPERIMENTAL
// AVR PGM basic_string tests

#if EXP_PGM_STRING
#include <estd/exp/pgm/string.h>


const char test1[] PROGMEM = "Hello PGM:";

// https://forum.arduino.cc/t/what-does-the-f-do-exactly/89384
// https://github.com/vancegroup-mirrors/avr-libc/blob/master/avr-libc/include/avr/pgmspace.h
// Looks like documented PSTR is a bit of a lie

//static estd::pgm_string s(PSTR("hello")); // FIX: Why don't you work??
//constexpr static estd::pgm_string pgm_s((const PROGMEM char*)"Hello AVR: ");
//constexpr static estd::pgm_string pgm_s(F("Hello AVR: "));
//static estd::pgm_string2 pgm_s5(PSTR("hello"));
// "statement-expressions are not allowed outside functions nor in template-argument lists"

static estd::basic_pgm_string<char, sizeof(test1) - 1> pgm_s3(test1);
static estd::pgm_string pgm_s4(test1);

struct Returner
{
    //static const char test2[] PROGMEM = "Hello AVR2";
    static estd::pgm_string value()
    {
        return { PSTR("(value)") };
    }

    static estd::layer2::string<> value2()
    {
        return "(value-2)";
    }
};

namespace avr {

void loop1(estd::layer1::string<64>& name)
{
    int v = pgm_s4 == "Hello PGM:";
    char c = pgm_s4[0];

    // FIX: 'v' as a bool is not getting expanded to true/false
    cout << F("loop: v=") << v << ", c=" << c << estd::endl;


    estd::pgm_string pgm_s(PSTR("(value-inline)"));
    estd::pgm_string pgm_s2 = Returner::value();

    name += pgm_s4;
    name += Returner::value();

    //name += Returner::value2();

    //name += F("(value)");
}


void loop2()
{
    estd::pgm_string pgm_s(PSTR("(value-inline)"));

    cout << pgm_s;
    cout << pgm_s3;
    cout << Returner::value();
    cout << estd::endl;
    //cout << Returner::value2();
}


}


#endif
