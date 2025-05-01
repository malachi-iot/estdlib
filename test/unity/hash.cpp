#include "unit-test.h"
#include "test-data.h"

#include <estd/internal/functional/hash.h>

static void test_fnv1a()
{

}

#ifdef ESP_IDF_TESTING
TEST_CASE("expected", "[expected]")
#else
void test_expected()
#endif
{
    RUN_TEST(test_fnv1a);
}

