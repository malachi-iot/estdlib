#include <Arduino.h>

#include <estd/string.h>

void setup()
{
    Serial.begin(115200);

    estd::layer1::string<64> name;

    name = "Mickey";

    Serial.println(F("hi"));
    
    Serial << "Hello: ";
    Serial << name << F(" Mouse");
}


void loop()
{

}