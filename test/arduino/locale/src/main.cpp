#include <Arduino.h>

#include <estd/ostream.h>
#include <estd/locale.h>

static estd::arduino_ostream cout(Serial);

using namespace estd;

void setup()
{
    Serial.begin(115200);
}

// NOTE: It feels odd that ostream flavor uses more, since even though
// it's fatter, the price (I thought) was already paid.  It makes sense
// optimizer would have easier time with isolated num_put, though
// atmega32u4, USE_WIDTH=0
// USE_NUM_PUT (0) = 4102
// USE_NUM_PUT (1) = 3890
// atmega32u4, USE_WIDTH=1
// USE_NUM_PUT (0) = 4126
// USE_NUM_PUT (1) = 3988
#define USE_NUM_PUT 1
#define USE_WIDTH 1

void loop()
{
    static unsigned counter = 0;

#if USE_NUM_PUT
    ios_base fmt;

    fmt.setf(ios_base::hex | ios_base::uppercase, ios_base::basefield);
#if USE_WIDTH
    fmt.width(4);
#endif

    char val[32];
    char* last;

    last = estd::num_put<char, char*>::put(val, fmt, '0', ++counter);
    *last = 0;

    cout << F("Hello World: ") << val << estd::endl;
#else
#if USE_WIDTH
    cout.width(4);
    cout.fill('0');
#endif
    cout.setf(ios_base::hex | ios_base::uppercase, ios_base::basefield);
    cout << F("Hello World: ") << ++counter << estd::endl;
#endif

    delay(1000);
}
