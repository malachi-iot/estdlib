#include <Arduino.h>

#include <estd/ostream.h>
#include <estd/locale.h>

static estd::arduino_ostream cout(Serial);

using namespace estd;

void setup()
{
    Serial.begin(115200);
}

// 17JUN24 NOTE: So far num_put uses 150b less ROM on AVR.  However, width() code hasn't
// been reworked for ostream just yet
// USE_NUM_PUT (0) = 4086
// USE_NUM_PUT (1) = 3918
#define USE_NUM_PUT 1

#if USE_NUM_PUT
using num_put = internal::num_put<char, char*>;
#endif

void loop()
{
    static unsigned counter = 0;

#if USE_NUM_PUT
    ios_base fmt;

    fmt.setf(ios_base::hex | ios_base::uppercase, ios_base::basefield);

    char val[32];
    char* last;

    last = num_put::put(val, fmt, ' ', ++counter);
    *last = 0;

    cout << F("Hello World: ") << val << estd::endl;
#else
    cout << F("Hello World: ") << ++counter << estd::endl;
#endif

    delay(1000);
}
