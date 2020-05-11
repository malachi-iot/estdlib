/**
 * @file
 */

#include "unit-test.h"

//#include <estd/map.h>

void test_layer1_map()
{

}

#ifdef ESP_IDF_TESTING
TEST_CASE("map tests", "[map]")
#else
void test_map()
#endif
{
    RUN_TEST(test_layer1_map);
}

