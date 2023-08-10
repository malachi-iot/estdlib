#include "unit-test.h"

#include <estd/tuple.h>

using namespace estd;
using namespace estd::test;

static void test_tuple_1()
{
    tuple<float, float> v(2.0, 3.0);

    TEST_ASSERT_EQUAL(2, get<0>(v));
    TEST_ASSERT_EQUAL(3, get<1>(v));
}


static void test_tuple_duplicate()
{
    tuple<
        Templated<0>,
        Templated<0>,
        Templated<0> > tuple3;

    get<0>(tuple3).val1 = 1;
    get<1>(tuple3).val1 = 2;
}

#ifdef ESP_IDF_TESTING
TEST_CASE("tuple tests", "[tuples]")
#else
void test_tuple()
#endif
{
    RUN_TEST(test_tuple_1);
    RUN_TEST(test_tuple_duplicate);
}