#include <estd/units.h>

#include "unit-test.h"

using namespace estd;

#ifdef ESP_IDF_TESTING
TEST_CASE("units (percent, bytes)", "[units]")
#else
void test_units()
#endif
{
    units::percent<int16_t> v1(5);

    v1 = v1 * 5;
}