#include "unit-test.h"

#include <estd/cstddef.h>

static void test_byte_shift()
{
    estd::byte value = estd::to_byte(1);

    value <<= 1;

    TEST_ASSERT_EQUAL_HEX8(2, estd::to_integer<int>(value));
}


void test_cstddef()
{
	RUN_TEST(test_byte_shift);
}