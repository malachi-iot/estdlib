#include <Arduino.h>

#include <estd/ostream.h>
#include <estd/string.h>

#if __AVR__
#include <estd/exp/pgm_string.h>
#include <estd/exp/pgm/string.h>

const char test1[] PROGMEM = "Hello AVR:";

//static estd::pgm_string s(PSTR("hello")); // FIX: Why don't you work??
//constexpr static estd::pgm_string pgm_s((const PROGMEM char*)"Hello AVR: ");
//constexpr static estd::pgm_string pgm_s(F("Hello AVR: "));
constexpr static estd::pgm_string pgm_s(test1);
constexpr static estd::basic_pgm_string<sizeof(test1) - 1> pgm_s3(test1);
static estd::basic_pgm_string2<> pgm_s4(test1);

struct Returner
{
    //static const char test2[] PROGMEM = "Hello AVR2";
    static estd::pgm_string value()
    {
        return { PSTR("(value)") };
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
    //estd::pgm_string pgm_s2(PSTR("(value)"));
    estd::pgm_string pgm_s2 = Returner::value();

    //name += pgm_s;
    name += pgm_s4;
    //name += Returner::value();
    //name += F("(value)");
#endif

#if USE_IOS
#if __AVR__
    cout << pgm_s;
    cout << pgm_s4; // NOTE: Uses a lot more memory than pgm_s
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