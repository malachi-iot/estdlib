#include "unit-test.h"
#include "test-data.h"

#include <estd/expected.h>

// FIX: File not yet picked up by anyone

static void test_expected_1()
{
}

#ifdef ESP_IDF_TESTING
TEST_CASE("expected", "[expected]")
#else
void test_expected()
#endif
{
    RUN_TEST(test_expected_1);
}

