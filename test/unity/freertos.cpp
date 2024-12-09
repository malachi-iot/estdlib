#include "unit-test.h"

#include <estd/port/identify_platform.h>

#ifdef ESTD_OS_FREERTOS

#include <estd/port/freertos/wrapper/task.h>

using namespace estd::freertos::wrapper;

static void test_task1(void* p)
{

}

static void test_static_task_wrapper()
{
    // DEBT: Do a malloc for these in SPIRAM for ESP32
    static task::static_type storage;
    static unsigned char stack[2048];

    task t = task::create_static(test_task1, "unity:freertos1",
        sizeof stack / 4, nullptr, 3, stack, &storage);

    t.suspend();
    t.resume();
}

#ifdef ESP_IDF_TESTING
TEST_CASE("freertos tests", "[freertos]")
{
    RUN_TEST(test_static_task_wrapper);
}
#endif

#endif
