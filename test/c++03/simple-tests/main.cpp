#include <iostream>

#include <estd/string.h>
#include <estd/ratio.h>

#include <unity.h>
#include <unit-test.h>

using namespace std;

typedef estd::ratio<1, 100> ratio1;
typedef estd::ratio<1, 30> ratio2;

typedef estd::ratio_divide<ratio1, ratio2> divided;

void test();
void test_chrono();
void test_tuple();

// +++ Unity specific
void setUp (void) {}
void tearDown (void) {}
// ---

int main()
{
    estd::layer1::string<20> test_str = "hi2u";

    cout << "Hello World: " << test_str << endl;
    cout << "Ratio: " << divided::num << '/' << divided::den << endl;

    test();
#if defined(FEATURE_CPP_VARIADIC)
    test_chrono();
    test_tuple();
#endif

    UNITY_BEGIN();
    test_align();
    test_array();
    test_chrono();
    test_cstddef();
    test_optional();
    test_queue();
    test_streambuf();
    test_string();
    UNITY_END();

    return 0;
}
