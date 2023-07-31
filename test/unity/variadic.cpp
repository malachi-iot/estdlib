#include "unit-test.h"
#include "test-data.h"

#include <estd/internal/variadic.h>

static void test_vtypes_1()
{

}

#ifdef ESP_IDF_TESTING
TEST_CASE("variadic", "[variadic]")
#else
void test_variadic()
#endif
{
    RUN_TEST(test_vtypes_1);
}
