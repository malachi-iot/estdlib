#include "unit-test.h"

#include <estd/optional.h>


static void test_optional_default()
{
    estd::optional<const char*> o;

    TEST_ASSERT_FALSE(o.has_value());
}


static void test_optional_charptr()
{
    estd::optional<const char*> o = "hello";

    TEST_ASSERT(o.has_value());
}


static void test_optional_int()
{
    estd::optional<int> o;

    TEST_ASSERT(!o.has_value());

    o = 10;
}

static void test_optional_layer1()
{
    estd::layer1::optional<int, -5> o;

    TEST_ASSERT(!o.has_value());
    TEST_ASSERT_EQUAL(-5, o.value());

    o = 10;

    TEST_ASSERT(o.has_value());
    TEST_ASSERT_EQUAL(10, o.value());
}



#ifdef ESP_IDF_TESTING
TEST_CASE("estd::optional", "[optional]")
#else
void test_optional()
#endif
{
    RUN_TEST(test_optional_default);
    RUN_TEST(test_optional_charptr);
    RUN_TEST(test_optional_int);
    RUN_TEST(test_optional_layer1);
}