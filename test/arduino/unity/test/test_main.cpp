#include <Arduino.h>

#include <unity.h>

#include <estd/cstddef.h>
#include "unity/unit-test.h"


void setup()
{
    // delay generally recommended by:
    // https://docs.platformio.org/en/stable/plus/unit-testing.html
    delay(5000);

    Serial.begin(9600);
    Serial.println("setup: begin");

    UNITY_BEGIN();

    test_align();
    test_array();
    test_chrono();
    test_cpp();
    test_cstddef();
    test_limits();
    test_map();
    test_optional();
    test_queue();
    test_streambuf();
    test_string();
    test_thread();

    UNITY_END();

    Serial.println("setup: end");
}


// Just to indicate we're not dead, we blink
void loop()
{
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}
