#include <Arduino.h>

#include <unity.h>

#include <estd/cstddef.h>
#include "unity/unit-test.h"

void setup()
{
    UNITY_BEGIN();

    test_chrono();
    test_cstddef();
    test_string();
    test_thread();
    test_map();

    UNITY_END();
}


void loop() {}
