#include "unit-test.h"
#include "test-data.h"

#include <estd/variant.h>

using namespace estd;

static void test_variant_1()
{
#if __cplusplus >= 201103L
    variant<int, const char*> v;

    TEST_ASSERT_TRUE(holds_alternative<int>(v));

    v = "hello";

    TEST_ASSERT_FALSE(holds_alternative<int>(v));
    TEST_ASSERT_TRUE(holds_alternative<const char*>(v));

    v = 7;

    TEST_ASSERT_TRUE(holds_alternative<int>(v));
    TEST_ASSERT_EQUAL(7, get<0>(v));
#endif
}


static void test_variant_nontrivial()
{
#if __cplusplus >= 201103L
    int counter = 0;
    variant<int, estd::test::NonTrivial> v(
        in_place_index_t<1>{}, 7, [&]{++counter;}), v2;

    v2 = std::move(v);

    // FIX: Does a copy, not a move, and doesn't call dtor like it should
    // (remember there's no assignment operator for this guy)
    //TEST_ASSERT_EQUAL(1, counter);
    TEST_ASSERT_EQUAL(7, get<1>(v2).code_);
    TEST_ASSERT_TRUE(get<1>(v2).initialized_);
    //TEST_ASSERT_FALSE(get<1>(v2).copied_);
    //TEST_ASSERT_TRUE(get<1>(v2).moved_);
#endif
}


#ifdef ESP_IDF_TESTING
TEST_CASE("variant", "[variant]")
#else
void test_variant()
#endif
{
    RUN_TEST(test_variant_1);
    RUN_TEST(test_variant_nontrivial);
}

