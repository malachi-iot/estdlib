#include <estd/units.h>

#include "unit-test.h"

using namespace estd;

#ifdef ESP_IDF_TESTING
TEST_CASE("units (percent, bytes)", "[units]")
#else
void test_units()
#endif
{
    {
        units::percent<int16_t> v1(5);
        units::percent<int> v2(5);

        // A bunch of misc things to poke and prod common_type and friends
        v1 = v1 * 5;
        v2 = v1 * 5;

        TEST_ASSERT_EQUAL(125, v2.count());

        v2 = v2 * 5;
        v2 = v2 + v1;
    }
    {
        // FIX: Now we are too permissive, this shouldn't compile
        units::percent<uint32_t> v1(5);
        units::percent<int16_t> v2(v1);
    }
}