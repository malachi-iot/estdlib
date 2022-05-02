#include "unit-test.h"

#include <estd/span.h>

static void test_span_1()
{
    char buf[128] = "ABC";
    estd::span<char> span(buf);

#if UNITY_VERSION < 0x200
    TEST_ASSERT_EQUAL('A', span[0]);
#else
    TEST_ASSERT_EQUAL_CHAR('A', span[0]);
#endif

    TEST_ASSERT_LESS_OR_EQUAL(sizeof(int) * 2, sizeof(span));
}

#ifdef ESP_IDF_TESTING
TEST_CASE("span", "[span]")
#else
void test_span()
#endif
{
    RUN_TEST(test_span_1);
}

