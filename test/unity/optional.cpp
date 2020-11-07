#include "unit-test.h"

#include <estd/optional.h>


#ifdef ESP_IDF_TESTING
TEST_CASE("queue tests", "[queue]")
#else
void test_optional()
#endif
{
    estd::optional<const char*> d2;

}