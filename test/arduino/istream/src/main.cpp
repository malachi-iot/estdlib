#include <Arduino.h>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/string.h>

uint32_t start_ms;

void setup() 
{
    // TODO: this is for 32u4 only.  Change this for other targets
    Serial.begin(115200);
}

void loop() 
{
    estd::arduino_ostream cout(Serial);
    estd::arduino_istream cin(Serial);

    uint32_t now_ms = millis();

    long count = now_ms - start_ms;

    delay(1000);

    cout << F("Please input something: ");

    estd::layer1::string<128> buffer;

    cin >> buffer;
    cout << F("You input: ") << buffer << estd::endl;
}
