#include <iostream>

#include <estd/string.h>

#include <unity.h>
#include <unit-test.h>

using namespace std;

void test();
void test_tuple();

// +++ Unity specific
void setUp (void) {}
void tearDown (void) {}
// ---

int main()
{
    estd::layer1::string<20> test_str = "hi2u";

    cout << "Hello World: " << test_str << endl;

    test();
#if defined(FEATURE_CPP_VARIADIC)
    test_tuple();
#endif

    UNITY_BEGIN();
    test_align();
    test_array();
    test_chrono();
    test_cpp();
    test_cstddef();
    test_limits();
    test_optional();
    test_queue();
    test_ratio();
    test_span();
    test_streambuf();
    test_string();
    UNITY_END();

    return 0;
}
