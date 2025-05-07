#include <Arduino.h>

#define TEST_ENABLE 0

#include <estd/thread.h>
#include <estd/type_traits.h>
#include <estd/chrono.h>
#if TEST_ENABLE
#include <estd/string.h>
#include <estd/iostream.h>
#else
#endif

// NOTE: for 32u4, compile size is identical using TEST_CHRONO or not.  Nice!
// 1/20/2020: No longer the case:
//            32u4 compiles 92 bytes larger :(
//            attiny85 compiles 32 bytes larger :(
// 29JUN23:   Back again! same size!
#define TEST_CHRONO

using namespace estd::chrono;
using namespace estd::chrono_literals;

// DEBT: Make a test flavor of this that excercises std::steady_clock
// However, arduino_clock is 100% appropriate here
using steady_clock = estd::chrono::arduino_clock;

steady_clock::time_point start;
uint32_t start_ms;

void setup() 
{
    // TODO: this is for 32u4 only.  Change this for other targets
    Serial.begin(115200);

#ifdef TEST_CHRONO
    // arduini millis() is easy enough, but chrono compatibility is helpful for
    // assisting libraries
    start = steady_clock::now();
#else
    start_ms = millis();
#endif
}

void loop() 
{
#if TEST_ENABLE
    estd::arduino_ostream cout(Serial);

#ifdef TEST_CHRONO
    steady_clock::time_point now = steady_clock::now();

    long count = duration_cast<milliseconds>(now - start).count();

    estd::this_thread::sleep_for(1000ms);
#else
    uint32_t now_ms = millis();

    long count = now_ms - start_ms;

    delay(1000);
#endif

    // interestingly, code size varies along with this buffer size
    estd::layer1::string<128> buffer = "Hello";

    cout << (buffer += F(" world")) << '!' << estd::endl;
    cout << F("hi2u ") << count << estd::endl;
#endif
}
