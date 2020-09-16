#include "unit-test.h"

#include <estd/queue.h>
#include <estd/mutex.h>

static void test_queue_1()
{

}

#ifdef ESP_IDF_TESTING
TEST_CASE("queue tests", "[queue]")
#else
void test_queue()
#endif
{
    RUN_TEST(test_queue_1);
}
