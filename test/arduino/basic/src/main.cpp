#include <Arduino.h>

#include <estd/string.h>
#include <estd/chrono.h>

void setup() 
{
    estd::layer1::string<128> buffer;

    buffer += "hello";

    // doesn't work yet, overloads don't know what to do with __FlashStringHelper*
    buffer += F(" world");
    buffer += '!';

    Serial.print(buffer.data());
}

void loop() 
{
    // arduini millis() is easy enough, but chrono compatibility is helpful for
    // assisting libraries
    estd::chrono::steady_clock::now();
}
