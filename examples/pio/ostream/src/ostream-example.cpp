#include <Arduino.h>

#include <estd/ostream.h>


static estd::arduino_ostream cout(Serial);

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    static int counter = 0;

    cout << "hi2u" << ++counter << estd::endl;
    delay(1000);
}