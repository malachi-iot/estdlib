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
    static unsigned counter;
    cout << F("sim(") << ++counter << F(")");
    cout << estd::endl;

    delay(1000);
}
