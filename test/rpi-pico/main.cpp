#include <unity.h>

#include <unit-test.h>

void setUp (void) {}
void tearDown (void) {}


int main()
{
    UNITY_BEGIN();
    test_cpp();
    test_cstddef();
    test_limits();
    test_optional();
    test_ratio();
    test_span();
    test_string();
    test_tuple();
    UNITY_END();

    return 0;
}