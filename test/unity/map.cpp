/**
 * @file
 */

#include "unit-test.h"

#include <estd/map.h>

void test_layer1_map()
{
    // adapted from catch.hpp unit tests
#if __cpp_initializer_lists
    estd::layer1::map<int, int, 4> map =
    {
        { 1, 77 },
        { 3, 78 },
        { 5, 79 },
        { 7, 80 }
    };

    auto result = map[3];

    TEST_ASSERT_EQUAL(78, *result);
#endif
}

#ifdef ESP_IDF_TESTING
TEST_CASE("map tests", "[map]")
#else
void test_map()
#endif
{
    RUN_TEST(test_layer1_map);
}

