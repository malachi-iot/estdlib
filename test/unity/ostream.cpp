#include "unit-test.h"

#include <estd/ostream.h>

// TODO: Bug during esp-idf int -> str conversion, dig into that here
// TODO: Move rpi pico ostream tests into here

static void test_ostringstream()
{

}

#ifdef ESP_IDF_TESTING
TEST_CASE("ostream", "[ostream]")
#else
void test_ostream()
#endif
{
    RUN_TEST(test_ostringstream);
}