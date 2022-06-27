#include "unit-test.h"

#include <estd/tuple.h>

using namespace estd::experimental;

static void test_tuple_1()
{
    tuple<float, float> v(2.0, 3.0);

    TEST_ASSERT_EQUAL(2, get<0>(v));
    TEST_ASSERT_EQUAL(3, get<1>(v));
}

#ifdef ESP_IDF_TESTING
TEST_CASE("tuple tests", "[tuples]")
#else
void test_tuple()
#endif
{
    RUN_TEST(test_tuple_1);
}