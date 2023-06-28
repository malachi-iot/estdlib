#include <Arduino.h>

#include <unity.h>

#include <estd/cstddef.h>
#include "unity/unit-test.h"

#ifdef LED_BUILTIN
CONSTEXPR static unsigned LED_PIN = LED_BUILTIN;
#endif


void setup()
{
    // delay generally recommended by:
    // https://docs.platformio.org/en/stable/plus/unit-testing.html
    delay(5000);

#ifdef LED_BUILTIN
    pinMode(LED_PIN, OUTPUT);
#endif

    Serial.begin(9600);
    Serial.println("setup: begin");

    UNITY_BEGIN();

    // DEBT: Consolidate this with the other explicit unity caller

    test_align();
    test_array();
    test_chrono();
    test_cpp();
    test_cstddef();
    test_expected();
    test_functional();
    test_limits();
    test_locale();
    test_map();
    test_optional();
    test_ostream();
    test_queue();
    test_ratio();
    test_span();
    test_streambuf();
    test_string();
    test_thread();
    test_tuple();
    test_variant();

    UNITY_END();

    Serial.println("setup: end");
}


// Just to indicate we're not dead, we blink
void loop()
{
#ifdef LED_BUILTIN
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(500);
#endif
}
