#include <Arduino.h>

// already defined
//#define ARDUINO

//#include <estd/vector.h>
#include <estd/string.h>

void setup() 
{
    estd::layer1::string<128> buffer;

    buffer += "hello";

    // doesn't work yet, overloads don't know what to do with __FlashStringHelper*
    buffer += F(" world");

    Serial.print(buffer.lock());
    buffer.unlock();
}

void loop() {}
