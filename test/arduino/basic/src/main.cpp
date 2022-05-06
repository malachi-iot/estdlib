#include <Arduino.h>

#include <estd/string.h>
#include <estd/chrono.h>
#include <estd/queue.h>
#include <estd/thread.h>

#include <estd/port/arduino/streambuf.h>
#include <estd/ostream.h>

// NOTE: for 32u4, compile size is identical using TEST_CHRONO or not.  Nice!
// 1/20/2020: No longer the case:
//            32u4 compiles 92 bytes larger :(
//            attiny85 compiles 32 bytes larger :(
#define TEST_CHRONO

using namespace estd::chrono;

steady_clock::time_point start;
uint32_t start_ms;

estd::layer1::queue<uint8_t, 16> test_queue;

typedef estd::internal::basic_ostream<estd::arduino_streambuf> ostream;

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
    ostream cout(&Serial);

    cout << "Hi2u estd" << estd::endl;

#ifdef TEST_CHRONO
    steady_clock::time_point now = steady_clock::now();

    long count = duration_cast<milliseconds>(now - start).count();

    estd::this_thread::sleep_for(milliseconds(1000));
#else
    uint32_t now_ms = millis();

    long count = now_ms - start_ms;

    delay(1000);
#endif

    // interestingly, code size varies along with this buffer size
    estd::layer1::string<128> buffer;

    buffer += "hello";

    // doesn't work yet, overloads don't know what to do with __FlashStringHelper*
    buffer += F(" world");
    buffer += '!';

    Serial.print(buffer.data());

    Serial.println(count);
}
