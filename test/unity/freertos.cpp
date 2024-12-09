#include "unit-test.h"

#include <estd/port/identify_platform.h>

#ifdef ESTD_OS_FREERTOS

#include <estd/port/freertos/wrapper/task.h>

using namespace estd::freertos::wrapper;

volatile bool test_task_completed = false;

static void test_task1(void* p)
{
    test_task_completed = true;

    // It seems the task may need minimum one context switch otherwise
    // FreeRTOS reports an improper task return
    vTaskDelay(10);

    vTaskDelete(nullptr);
}

static void test_static_task_wrapper()
{
    // DEBT: Do a malloc for these in SPIRAM for ESP32
    static StaticTask_t storage;
    static portSTACK_TYPE stack[2048];

    task t = task::create_static(test_task1, "unity:freertos1",
        (sizeof stack) / sizeof(portSTACK_TYPE), nullptr, 3, stack, &storage);

    t.suspend();
    t.resume();

    while(!test_task_completed)    vTaskDelay(10);

    test_task_completed = false;

    static task::storage<2048> storage2;

    t = task::create_static(test_task1, "unity:freertos2", nullptr, 3, &storage2);

    while(!test_task_completed)    vTaskDelay(10);
}

#ifdef ESP_IDF_TESTING
TEST_CASE("freertos tests", "[freertos]")
{
    RUN_TEST(test_static_task_wrapper);
}
#endif

#endif
