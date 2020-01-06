#include <Arduino.h>

#include <unity.h>

#include <estd/cstddef.h>

void test_1()
{
    estd::byte test_byte = estd::to_byte(1);

    test_byte <<= 1;
}


void setup()
{
    UNITY_BEGIN();

    RUN_TEST(test_1);
}


void loop() {}
