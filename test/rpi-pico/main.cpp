#include <stdio.h>
#include <pico/stdlib.h>

#include <estd/string.h>

#include <unity.h>

#include <unit-test.h>

void setUp (void) {}
void tearDown (void) {}

void proving();


int main()
{
    stdio_init_all();

    UNITY_BEGIN();
    test_align();
    test_array();
    test_chrono();
    test_cpp();
    test_cstddef();
    test_limits();
    test_locale();
    test_optional();
    test_queue();
    test_ratio();
    test_span();
    test_streambuf();
    test_string();
    test_thread();
    test_tuple();
    UNITY_END();

    proving();

    while (true) {
        static int counter = 0;

        estd::layer1::string<32> s = "Hello, unit test ";

        s += estd::to_string(++counter);

        puts(s.data());

        sleep_ms(5000);
    }

    return 0;
}