#include "unit-test.h"

#include <estd/cstddef.h>

static void test_byte_shift()
{
    estd::byte value = estd::to_byte(1);

    value <<= 1;

    TEST_ASSERT_EQUAL_HEX8(2, estd::to_integer<int>(value));
}

struct nullptr_t_tester
{
    int val;

    nullptr_t_tester() {}

    nullptr_t_tester(estd::nullptr_t) : val(0) {}

    nullptr_t_tester(int val) : val(val) {}
};


static void test_nullptr_t()
{
    {
        nullptr_t_tester v;

        v.val = 5;

        TEST_ASSERT_EQUAL(5, v.val);
    }

    {
        estd::nullptr_t n;
        nullptr_t_tester v(n);

        TEST_ASSERT_EQUAL(0, v.val);
    }

    {
        nullptr_t_tester v(estd::nullptr_t{});

        TEST_ASSERT_EQUAL(0, v.val);
    }

    {
        nullptr_t_tester v(4);

        TEST_ASSERT_EQUAL(4, v.val);
    }
}


#ifdef ESP_IDF_TESTING
TEST_CASE("cstddef tests", "[cstddef]")
#else
void test_cstddef()
#endif
{
	RUN_TEST(test_byte_shift);
    RUN_TEST(test_nullptr_t);
}

