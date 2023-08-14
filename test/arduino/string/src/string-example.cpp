#include <Arduino.h>


#include <estd/string.h>

#include "main.h"


#define USE_IOS 1

#if USE_IOS
estd::arduino_ostream cout(Serial);
#endif


void setup()
{
    Serial.begin(115200);

    while(!Serial);
}


void loop()
{
    static int counter = 0;
    
    estd::layer1::string<64> name;

    name = "Mickey";
    name += ' ';
    name += "Mouse";

#if EXP_PGM_STRING
    avr::loop1(name);
#endif

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

#if EXP_PGM_STRING
    avr::loop2();
#endif
    
    delay(1000);
}
