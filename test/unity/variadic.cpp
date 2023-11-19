#include "unit-test.h"
#include "test-data.h"

#include <estd/internal/variadic.h>
#include <estd/internal/variadic/selector.h>

using namespace estd;

static void test_vtypes_1()
{

}


static void test_constructible_selector()
{
    //using types = variadic::types<int, const char*>;
    using _selector = internal::constructible_selector<const char*>;
    using selected = variadic::v2::selector<_selector, int, const char*>::selected;
    
    TEST_ASSERT_EQUAL(1, selected::first::index);
#if __AVR__
    // DEBT: https://github.com/malachi-iot/estdlib/issues/7
    TEST_ASSERT_EQUAL(2, selected::size);
#else
    TEST_ASSERT_EQUAL(1, selected::single::index);
#endif
}


#ifdef ESP_IDF_TESTING
TEST_CASE("variadic", "[variadic]")
#else
void test_variadic()
#endif
{
    RUN_TEST(test_constructible_selector);
    RUN_TEST(test_vtypes_1);
}
