#include "unit-test.h"
#include "test-data.h"

#include <estd/variant.h>

using namespace estd;

static void test_variant_storage_1()
{
	typedef internal::variant_storage<estd::monostate, int> vs_type;
	
	vs_type::size_type index = 0;
	vs_type vs;
	int val = 7;
	
	vs.assign_or_init(&index, val); 
}

static void test_variant_1()
{
#if __cplusplus >= 201103L
    variant<int, const char*> v;

    TEST_ASSERT_TRUE(holds_alternative<int>(v));

    // FIX: hangs AVR - we suspect our is_convertible and friends of having a bug
    // which selects the wrong index
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
#if FEATURE_STD_FUNCTIONAL
    int counter = 0;
#endif
    variant<int, estd::test::NonTrivial> v(
        in_place_index_t<1>{}, 7,
#if FEATURE_STD_FUNCTIONAL
        [&]{++counter;}),
#else
        true),
#endif
        v2;

    estd::test::NonTrivial& v3 = get<1>(v);

    v2 = std::move(v);

    // No dtor_fn call yet since v2 was unassigned and a move
    // from nulls out dtor_fn
    // (remember also there's no assignment operator for this guy)
#if FEATURE_STD_FUNCTIONAL
    TEST_ASSERT_EQUAL(0, counter);
#endif
    TEST_ASSERT_EQUAL(7, get<1>(v2).code_);
    TEST_ASSERT_TRUE(get<1>(v2).initialized_);
    TEST_ASSERT_FALSE(get<1>(v2).copied_);
    TEST_ASSERT_TRUE(get<1>(v2).moved_);
    TEST_ASSERT_FALSE(get<1>(v2).moved_from_);

    // FIX: Fails on AVR
    TEST_ASSERT_TRUE(v3.moved_from_);

    v2 = 0;

#if FEATURE_STD_FUNCTIONAL
    // v2 assignment now destroyes non trivial and activates
    // dtor_fn
    TEST_ASSERT_EQUAL(1, counter);
#endif

#endif
}


#ifdef ESP_IDF_TESTING
TEST_CASE("variant", "[variant]")
#else
void test_variant()
#endif
{
    RUN_TEST(test_variant_storage_1);
    RUN_TEST(test_variant_1);
    RUN_TEST(test_variant_nontrivial);
}

