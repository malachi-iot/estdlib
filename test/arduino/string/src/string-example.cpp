#include <Arduino.h>

#include <estd/ostream.h>
#include <estd/string.h>

#if __AVR__
#include <estd/exp/pgm_string.h>

//static estd::pgm_string s(PSTR("hello")); // FIX: Why don't you work??
static estd::pgm_string pgm_s((const PROGMEM char*)"hello");
#endif

void setup()
{
    Serial.begin(115200);
}


#define USE_IOS 1

#if USE_IOS
static estd::arduino_ostream cout(Serial);
#endif

void loop()
{
    static int counter = 0;

    estd::layer1::string<64> name;

    name = "Mickey";
    name += ' ';
    name += "Mouse";

    //name += pgm_s;

#if USE_IOS
    cout << F("Hello: ");
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