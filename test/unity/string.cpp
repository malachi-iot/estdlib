#include "unit-test.h"

#include <estd/string.h>

template <class TString>
void _test_string()
{
    const char* test_str = "hi2u";

    TString s = test_str;

    TEST_ASSERT_EQUAL_STRING(test_str, s.data());
}

void test_layer1_string()
{
    _test_string<estd::layer1::string<32, true> >();
    // FIX: non-null-terminated has an issue
    //_test_string<estd::layer1::string<32, false> >();
}


void test_layer2_string()
{
    _test_string<estd::layer2::const_string>();
}


void test_string()
{
    RUN_TEST(test_layer1_string);
    RUN_TEST(test_layer2_string);
}