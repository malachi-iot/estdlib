#include "unit-test.h"
#include "test-data.h"

#include <estd/span.h>

using namespace test;

static void test_span_1()
{
    estd::span<char> span(span_buf);

#if UNITY_VERSION < 0x200
    TEST_ASSERT_EQUAL('A', span[0]);
#else
    TEST_ASSERT_EQUAL_CHAR('A', span[0]);
#endif

    TEST_ASSERT_LESS_OR_EQUAL(sizeof(char*) * 2, sizeof(span));
}

static void test_span_copy()
{
    estd::span<char> span1(span_buf), span2 = span1;

    TEST_ASSERT_EQUAL('A', span1[0]);
}

#ifdef ESP_IDF_TESTING
TEST_CASE("span", "[span]")
#else
void test_span()
#endif
{
    RUN_TEST(test_span_1);
    RUN_TEST(test_span_copy);
}

