#include <estd/bit.h>

#include "unit-test.h"

#ifdef ESP_IDF_TESTING
TEST_CASE("bit/endian tests", "[bit]")
#else
void test_bit()
#endif
{
    constexpr auto vu16 = estd::byteswap<uint16_t>(0x1234);
    constexpr auto vi32 = estd::byteswap<int32_t>(-500);

    TEST_ASSERT_EQUAL(0x3412, vu16);
    TEST_ASSERT_EQUAL(-500, estd::byteswap(vi32));
}