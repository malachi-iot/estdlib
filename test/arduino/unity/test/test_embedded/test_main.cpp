#include <Arduino.h>

#include <unity.h>

#include <estd/cstddef.h>
#include "unity/unit-test.h"

#ifdef __AVR__
#include <avr/wdt.h>
#endif

#ifdef LED_BUILTIN
CONSTEXPR static unsigned LED_PIN = LED_BUILTIN;
#endif


void setup()
{
    Serial.begin(9600);

#ifdef __AVR__
    wdt_disable();
#endif

    // delay generally recommended by:
    // https://docs.platformio.org/en/stable/plus/unit-testing.html
    delay(5000);

#ifdef __AVR__
    wdt_enable(WDTO_8S);
#endif

    while(!Serial);

#ifdef LED_BUILTIN
    pinMode(LED_PIN, OUTPUT);
#endif

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
    test_type_traits();
    //test_variadic();
    //test_variant();

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

#ifdef __AVR__
    wdt_reset();
#endif
}
