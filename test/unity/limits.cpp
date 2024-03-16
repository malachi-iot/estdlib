#include "unit-test.h"

#include <estd/limits.h>

#undef min
#undef max

static void test_limits_1()
{
    TEST_ASSERT(estd::numeric_limits<int8_t>::is_signed);
    TEST_ASSERT_EQUAL(127, estd::numeric_limits<int8_t>::max());
}

static void test_limits_least64()
{
    TEST_ASSERT(estd::numeric_limits<estd::uint_least64_t>::is_signed);
    TEST_ASSERT_EQUAL(INT_LEAST64_MAX, estd::numeric_limits<uint_least64_t>::max());
}


#ifdef ESP_IDF_TESTING
TEST_CASE("limits tests", "[limits]")
#else
void test_limits()
#endif
{
    RUN_TEST(test_limits_1);
    RUN_TEST(test_limits_least64);
}
