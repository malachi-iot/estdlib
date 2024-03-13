#include "unit-test.h"

#include <estd/cstddef.h>
#include <estd/cstdint.h>

static void test_byte_shift()
{
    estd::byte value = estd::to_byte(1);

    value <<= 1;

    TEST_ASSERT_EQUAL_HEX8(2, estd::to_integer<int>(value));
}


static void test_variant_int_types()
{
}


static void test_nullptr_t()
{
    // TODO
}


#ifdef ESP_IDF_TESTING
TEST_CASE("cstddef tests", "[cstddef]")
#else
void test_cstddef()
#endif
{
	RUN_TEST(test_byte_shift);
    RUN_TEST(test_nullptr_t);
    RUN_TEST(test_variant_int_types);
}

