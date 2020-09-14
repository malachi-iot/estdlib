#include <Arduino.h>

#include <estd/string.h>
#include <estd/charconv.h>

void setup()
{
    Serial.begin(115200);
}


void loop()
{
    static int counter = 0;

    estd::layer2::const_string number = "1234";

    long value;

    estd::from_chars(number.data(), number.data() + number.size(), value);

    Serial << "Hello: ";
    Serial << F("Number - counter=") << counter++;

    Serial.println(F("!!"));
    
    delay(1000);
}