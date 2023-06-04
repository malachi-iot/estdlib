#include "unit-test.h"
#include "test-data.h"

#include <estd/internal/variant.h>

using namespace estd;

// NOTE: ADL doesn't seem to quite do what we expect with GCC 8.3.1 (rpi pico)
// so we include this namespace too
using namespace estd::internal;

static void test_variant_1()
{
    variant<int, const char*> v;

    TEST_ASSERT_TRUE(holds_alternative<int>(v));

    v = (const char*)"hello";

    TEST_ASSERT_FALSE(holds_alternative<int>(v));
    TEST_ASSERT_TRUE(holds_alternative<const char*>(v));

    v = 7;

    TEST_ASSERT_TRUE(holds_alternative<int>(v));
    TEST_ASSERT_EQUAL(7, get<0>(v));
}

#ifdef ESP_IDF_TESTING
TEST_CASE("variant", "[variant]")
#else
void test_variant()
#endif
{
    RUN_TEST(test_variant_1);
}

