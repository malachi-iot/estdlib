#include <estd/cmath.h>
#include <estd/ostream.h>

using namespace estd;

static arduino_ostream cout(Serial);

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    static float counter;

    counter += 0.01;

    cout << F("sin(") << counter << F(")=") << estd::sin(counter);
    cout << estd::endl;

    delay(1000);
}
