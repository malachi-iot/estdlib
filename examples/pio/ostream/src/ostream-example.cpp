#include <Arduino.h>

#include <estd/ostream.h>


static estd::arduino_ostream cout(Serial);

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    static unsigned counter = 0;

    cout << F("Hello World: ") << ++counter << estd::endl;
    delay(1000);
}
