#include <Arduino.h>

#include <estd/string.h>
#include <estd/chrono.h>
#include <estd/queue.h>

// NOTE: for 32u4, compile size is identical using TEST_CHRONO or not.  Nice!
#define TEST_CHRONO

using namespace estd::chrono;

steady_clock::time_point start;
uint32_t start_ms;

estd::layer1::queue<uint8_t, 16> test_queue;

void setup() 
{
#ifdef TEST_CHRONO
    // arduini millis() is easy enough, but chrono compatibility is helpful for
    // assisting libraries
    start = steady_clock::now();
#else
    start_ms = millis();
#endif

    // interestingly, code size varies along with this buffer size
    estd::layer1::string<128> buffer;

    buffer += "hello";

    // doesn't work yet, overloads don't know what to do with __FlashStringHelper*
    buffer += F(" world");
    buffer += '!';

    Serial.print(buffer.data());
}

void loop() 
{
#ifdef TEST_CHRONO
    steady_clock::time_point now = steady_clock::now();

    auto count = duration_cast<milliseconds>(now - start).count();
#else
    uint32_t now_ms = millis();

    auto count = now_ms - start_ms;
#endif

    Serial.println(count);
}
