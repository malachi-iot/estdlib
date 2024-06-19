#include <Arduino.h>

#include <estd/ostream.h>
#include <estd/locale.h>

static estd::arduino_ostream cout(Serial);

using namespace estd;

void setup()
{
    Serial.begin(115200);
}

// USE_NUM_PUT (0) = 4094
// USE_NUM_PUT (1) = 3890
#define USE_NUM_PUT 1

void loop()
{
    static unsigned counter = 0;

#if USE_NUM_PUT
    ios_base fmt;

    fmt.setf(ios_base::hex | ios_base::uppercase, ios_base::basefield);

    char val[32];
    char* last;

    last = estd::num_put<char, char*>::put(val, fmt, ' ', ++counter);
    *last = 0;

    cout << F("Hello World: ") << val << estd::endl;
#else
    cout << F("Hello World: ") << ++counter << estd::endl;
#endif

    delay(1000);
}
