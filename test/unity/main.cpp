// Generic main, picked up by regular desktop targets

#include <unity.h>

#include "unit-test.h"

int main()
{
    UNITY_BEGIN();
    test_cpp();
    test_cstddef();
    test_limits();
    test_locale();
    test_optional();
    test_queue();
    test_ratio();
    test_span();
    test_tuple();
    test_variant();
    UNITY_END();

    return 0;
}