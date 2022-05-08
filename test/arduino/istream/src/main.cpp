#include <Arduino.h>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/string.h>

uint32_t start_ms;

void setup() 
{
    Serial.begin(115200);

    delay(1000);
}

void loop() 
{
    estd::arduino_ostream cout(Serial);
    estd::arduino_istream cin(Serial);

    uint32_t now_ms = millis();

    long count = now_ms - start_ms;

    cout << F("Please input something: ");

    estd::layer1::string<128> buffer;

    cin >> buffer;
    cout << F("You input: ") << buffer << estd::endl;
}
