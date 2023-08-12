#include <Arduino.h>

#include <estd/ostream.h>
#include <estd/string.h>

#if __AVR__
//#include <estd/exp/pgm_string.h>
#include <estd/exp/pgm/string.h>

const char test1[] PROGMEM = "Hello AVR:";

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

#endif

#define USE_IOS 1

#if USE_IOS
static estd::arduino_ostream cout(Serial);
#endif


void setup()
{
    Serial.begin(115200);

    while(!Serial);
}


void loop()
{
    static int counter = 0;

#if __AVR__
    int v = pgm_s4 == "Hello AVR:";
    char c = pgm_s4[0];

    // FIX: 'v' as a bool is not getting expanded to true/false
    cout << F("loop: v=") << v << ", c=" << c << estd::endl;
#endif

    estd::layer1::string<64> name;

    name = "Mickey";
    name += ' ';
    name += "Mouse";

#if __AVR__
    estd::pgm_string pgm_s(PSTR("(value-inline)"));
    estd::pgm_string pgm_s2 = Returner::value();

    name += pgm_s4;
    name += Returner::value();
    //name += Returner::value2();

    //name += F("(value)");
#endif

#if USE_IOS
#if __AVR__
    cout << pgm_s;
    cout << pgm_s3;
    cout << Returner::value();
    cout << estd::endl;
    //cout << Returner::value2();
#else
    cout << F("Hello: ");
#endif
    cout << name << F(" - counter=") << counter++;
#else
    Serial.print(F("Hello: "));
    Serial.print(name.data());
    Serial.print(F(" - counter="));
    Serial.print(counter++);
#endif

    Serial.println(F("!!"));
    
    delay(1000);
}
