#include "unit-test.h"
#include "test-data.h"

#include <estd/expected.h>

using namespace estd;

static void test_expected_void()
{
    expected<void, int> e;

    TEST_ASSERT_TRUE(e.has_value());
    TEST_ASSERT_EQUAL(sizeof(e), sizeof(int) * 2);
}

static void test_expected_int()
{
    expected<int, int> e, e2(in_place_t(), 7);

    TEST_ASSERT_TRUE(e.has_value());
    TEST_ASSERT_TRUE(e.value() == 0);
    TEST_ASSERT_EQUAL(sizeof(e), sizeof(int) * 2);

    TEST_ASSERT_EQUAL(7, *e2);
}

#ifdef ESP_IDF_TESTING
TEST_CASE("expected", "[expected]")
#else
void test_expected()
#endif
{
    RUN_TEST(test_expected_void);
    RUN_TEST(test_expected_int);
}

