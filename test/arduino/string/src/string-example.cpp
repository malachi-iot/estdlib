#include <Arduino.h>

#include <estd/string.h>

void setup()
{
    Serial.begin(115200);
}


#define USE_IOS 0


void loop()
{
    static int counter = 0;

    estd::layer1::string<64> name;

    name = "Mickey";
    name += ' ';
    name += "Mouse";

#if USE_IOS
    // FIX: Use arduino_stream here, this is now broken
    Serial << "Hello: ";
    Serial << name << F(" - counter=") << counter++;
#else
    Serial.print(F("Hello: "));
    Serial.print(name.data());
    Serial.print(F(" - counter="));
    Serial.print(counter++);
#endif

    Serial.println(F("!!"));
    
    delay(1000);
}