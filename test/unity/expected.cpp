#include "unit-test.h"
#include "test-data.h"

#include <estd/expected.h>

using namespace estd;

static void test_expected_1()
{
    expected<void, int> e;

    TEST_ASSERT_TRUE(e.has_value());
    TEST_ASSERT_EQUAL(sizeof(e), sizeof(int) * 2);
}

#ifdef ESP_IDF_TESTING
TEST_CASE("expected", "[expected]")
#else
void test_expected()
#endif
{
    RUN_TEST(test_expected_1);
}

