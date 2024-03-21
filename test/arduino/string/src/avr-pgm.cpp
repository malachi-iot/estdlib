#include "main.h"

// EXPERIMENTAL
// AVR PGM basic_string tests

#if EXP_PGM_STRING
#include <estd/exp/pgm/array.h>
#include <estd/exp/pgm/string.h>

static const estd::v0::test_container<int, 4> array1 PROGMEM { 0, 1, 2, 3 };

static const char test1[] PROGMEM = "Hello PGM:";

// https://forum.arduino.cc/t/what-does-the-f-do-exactly/89384
// https://github.com/vancegroup-mirrors/avr-libc/blob/master/avr-libc/include/avr/pgmspace.h
// Looks like documented PSTR is a bit of a lie

//static estd::pgm_string s(PSTR("hello")); // FIX: Why don't you work??
//constexpr static estd::pgm_string pgm_s((const PROGMEM char*)"Hello AVR: ");
//constexpr static estd::pgm_string pgm_s(F("Hello AVR: "));
//static estd::pgm_string2 pgm_s5(PSTR("hello"));
// "statement-expressions are not allowed outside functions nor in template-argument lists"

// Wishing for __flash keyword:
// https://community.platformio.org/t/avr-flash-flash-not-recognized-in-c/5454/9

static constexpr estd::basic_pgm_string<char, sizeof(test1) - 1> hello_explicit_len(test1);
static constexpr estd::pgm_string hello = test1;

struct Returner
{
    static estd::pgm_string value()
    {
        return { PSTR("(value)") };
    }

    static estd::layer2::basic_string<const char, 0> value2()
    {
        return "(value-2)";
    }

#if __cpp_inline_variables
    constexpr static const char value3_[] PROGMEM = "(value3)"; 

    static constexpr estd::pgm_string value3()
    {
        return { value3_ };
    }
#endif
};

namespace avr {

void loop1(estd::layer2::string<> name)
{
    int v = hello == "Hello PGM:";
    char c = hello[0];

    // FIX: 'v' as a bool is not getting expanded to true/false
    cout << F("loop: v=") << v << ", c=" << c << estd::endl;

    estd::pgm_string returner_value = Returner::value();

    name += hello;
    name += returner_value;

    //name += Returner::value2();

    //name += F("(value)");
    uint32_t dummy = array1[0];
    cout << "pgm_array: " << dummy << ", ";
    
    for(auto dummy2 : array1)
    {
        cout << dummy2 << ", ";
    }

    cout << estd::endl;
}


void loop2()
{
    const estd::pgm_string pgm_s(PSTR("(value-inline)"));

    cout << pgm_s;
    cout << hello_explicit_len;
    cout << Returner::value();
#if __cpp_inline_variables
    cout << F(", ");
    constexpr estd::pgm_string pgm_value3 = Returner::value3();
    cout << pgm_value3;
#endif
    cout << estd::endl;
    //cout << Returner::value2();
}


}


#endif
