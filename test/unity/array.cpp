#include "unit-test.h"

#include <estd/array.h>

static void test_layer1_array()
{
    estd::array<int, 10> a;

    TEST_ASSERT_EQUAL(10, a.size());
    TEST_ASSERT_EQUAL(sizeof(int) * 10, sizeof(a));
}

static void test_layer3_array()
{
    int _a[10];
    estd::legacy::layer3::array<int> a(_a);

    TEST_ASSERT_EQUAL(10, a.size());
}



#ifdef ESP_IDF_TESTING
TEST_CASE("array tests", "[array]")
#else
void test_array()
#endif
{
    RUN_TEST(test_layer1_array);
    RUN_TEST(test_layer3_array);
}
