#include <Arduino.h>

#include <estd/string.h>

void setup()
{
    Serial.begin(115200);
}


void loop()
{
    static int counter = 0;

    estd::layer1::string<64> name;

    name = "Mickey";
    name += ' ';
    name += "Mouse";

    Serial << "Hello: ";
    Serial << name << F(" - counter=") << counter++;

    Serial.println(F("!!"));
    
    delay(1000);
}