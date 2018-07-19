#include <Arduino.h>

#include <estd/string.h>
#include <estd/chrono.h>

using namespace estd::chrono;

void setup() 
{
    // arduini millis() is easy enough, but chrono compatibility is helpful for
    // assisting libraries
    //steady_clock::time_point time = steady_clock::now();

    estd::layer1::string<128> buffer;

    buffer += "hello";

    // doesn't work yet, overloads don't know what to do with __FlashStringHelper*
    buffer += F(" world");
    buffer += '!';

    Serial.print(buffer.data());
}

void loop() 
{
    steady_clock::time_point time = steady_clock::now();

    //auto count = duration_cast<milliseconds>(time - first).count();
}
