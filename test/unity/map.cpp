/**
 * @file
 */

#include "unit-test.h"

#include <estd/map.h>

void test_layer1_map()
{
    // copy/pasted from catch.hpp unit tests
    
    estd::layer1::map<int, int, 4> map =
    {
        { 1, 77 },
        { 2, 78 },
        { 3, 79 },
        { 4, 80 }
    };

    auto result = map[2];

    TEST_ASSERT_EQUAL_HEX8(78, *result);
}

#ifdef ESP_IDF_TESTING
TEST_CASE("map tests", "[map]")
#else
void test_map()
#endif
{
    RUN_TEST(test_layer1_map);
}

