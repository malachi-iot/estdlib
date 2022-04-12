#include "unit-test.h"

#include <estd/limits.h>

#undef min
#undef max

static void test_limits_1()
{
    TEST_ASSERT(estd::numeric_limits<int8_t>::is_signed);
    TEST_ASSERT_EQUAL(127, estd::numeric_limits<int8_t>::max());
}


#ifdef ESP_IDF_TESTING
TEST_CASE("limits tests", "[limits]")
#else
void test_limits()
#endif
{
    RUN_TEST(test_limits_1);
}
