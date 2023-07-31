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
    using selector = variadic::selector<_selector, int, const char*>;
    
    TEST_ASSERT_EQUAL(1, selector::first::index);
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
