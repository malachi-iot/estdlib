#include <estd/chrono.h>
#include <estd/ostream.h>
#include <estd/thread.h>

using namespace estd;

arduino_ostream cout(Serial);

void setup()
{
    Serial.begin(115200);
    while(!Serial);
}

void loop()
{
    auto now = chrono::steady_clock::now();

    // Neither of these work anyway
    //std::chrono::seconds s(std::chrono::milliseconds(1000));
    //std::chrono::seconds ms(estd::chrono::milliseconds(1000));

    now += estd::chrono::milliseconds(1000);
    now + estd::chrono::milliseconds(1000);

    estd::this_thread::sleep_for(estd::chrono::milliseconds(1000));
}
