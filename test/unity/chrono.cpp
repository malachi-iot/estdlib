/**
 * @file
 */

#include "unit-test.h"

#include <estd/chrono.h>

static void test_chrono_convert()
{
    estd::chrono::minutes m(2);

    estd::chrono::seconds s = estd::chrono::duration_cast<estd::chrono::seconds>(m);

    TEST_ASSERT_EQUAL(120, s.count());
}

static void test_chrono_subtract()
{
    estd::chrono::seconds s(10);
    estd::chrono::minutes m(2);

    estd::common_type<estd::chrono::seconds,estd::chrono::minutes>::type d;

    d = m - s;

    TEST_ASSERT_EQUAL_INT(110, d.count());

    estd::chrono::milliseconds ms(100);

    ms = d - ms;

    TEST_ASSERT_EQUAL_INT(110000 - 100,ms.count());
}

#ifdef ESP_IDF_TESTING
TEST_CASE("chrono tests", "[chrono]")
#else
void test_chrono()
#endif
{
    RUN_TEST(test_chrono_convert);
    RUN_TEST(test_chrono_subtract);
}

