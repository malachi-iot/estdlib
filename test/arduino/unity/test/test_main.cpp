#include <Arduino.h>

#include <unity.h>

#include <estd/cstddef.h>

void test_byte_to_integer()
{
    estd::byte test_byte = estd::to_byte(1);

    test_byte <<= 1;

    TEST_ASSERT_EQUAL(2, estd::to_integer<int>(test_byte));
}


void setup()
{
    UNITY_BEGIN();

    RUN_TEST(test_byte_to_integer);

    UNITY_END();
}


void loop() {}
