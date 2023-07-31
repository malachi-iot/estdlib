#include "unit-test.h"
#include "test-data.h"

#include <estd/internal/variadic.h>

#ifdef ESP_IDF_TESTING
TEST_CASE("variadic", "[variadic]")
#else
void test_variadic()
#endif
{
}
