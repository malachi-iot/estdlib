#include "unit-test.h"

#include <estd/ostream.h>
#include <estd/sstream.h>

using namespace estd;

// TODO: Move rpi pico ostream tests into here

static void test_ostringstream()
{
    // DEBT: Until we overhaul it, maxStringLength runtime fails when
    // it can't identify your integer type (yuck)
    unsigned len = internal::numeric_limits<int>::length<10>::value;

    // 16 bits minimum size expected
    TEST_ASSERT_GREATER_THAN(4, len);

    experimental::ostringstream<32> out;

    const int val1 = 123;

    out << val1;

    const char* s = out.rdbuf()->str().data();

    TEST_ASSERT_EQUAL_STRING("123", s);
}

#ifdef ESP_IDF_TESTING
TEST_CASE("ostream", "[ostream]")
#else
void test_ostream()
#endif
{
    RUN_TEST(test_ostringstream);
}