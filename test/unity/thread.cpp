#include "unit-test.h"

#include <estd/thread.h>
#include <estd/mutex.h>

#ifdef FEATURE_ESTD_THREADING

void test_thread_get_id()
{
    estd::thread::id id = estd::this_thread::get_id();
    
    // pretty fundamental, just makes sure we're interacting
    // with *something* consistent
    TEST_ASSERT_EQUAL(id, estd::this_thread::get_id());
}

// TODO: Do a simplistic timed mutex scenario so that we are reasonably
// sure we wait for one before doing the other
// NOTE: This is *not* in reference to timed_mutex
// https://en.cppreference.com/w/cpp/thread/timed_mutex
void test_lock_guard()
{
    estd::mutex m;
    estd::lock_guard<estd::mutex> _m(m);
}

#ifdef ESTD_OS_FREERTOS

template <bool is_static>
static void test_freertos_mutex_task(void* p)
{
    vTaskDelete(NULL);
}

// FIX: Incomplete unit test
template <bool is_static>
static void test_freertos_mutex_iteration(estd::freertos::mutex<is_static>& m)
{
    TEST_ASSERT_NOT_NULL(m.native_handle());

    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    m.lock();
    TEST_ASSERT_TRUE(m.unlock());

    xReturned = xTaskCreate(
        test_freertos_mutex_task<is_static>,
        "test_freertos_mutex",
        2048,
        &m,
        4,      // DEBT: Need a more clear or configurable priority
        &xHandle);

    TEST_ASSERT_EQUAL(pdPASS, xReturned);
}

static void test_freertos_mutex()
{
    // Dynamic (regular) mutex
    {
        estd::freertos::mutex<false> m;

        test_freertos_mutex_iteration(m);
    }

    // FIX: The binary flavor causes a lockup
    return;

    // Dynamic (regular) mutex, binary
    {
        estd::freertos::mutex<false> m(true);

        test_freertos_mutex_iteration(m);
    }
}

static void test_freertos_mutex_static()
{
    // Static mutex
    {
        estd::freertos::mutex<true> m;

        test_freertos_mutex_iteration(m);
    }

    return;

    // Static mutex, binary
    {
        estd::freertos::mutex<true> m(true);

        test_freertos_mutex_iteration(m);
    }
}

static void test_freertos_thread()
{

}


static void test_freertos()
{
    RUN_TEST(test_freertos_mutex);
    RUN_TEST(test_freertos_mutex_static);
    RUN_TEST(test_freertos_thread);
}
#endif


#ifdef ESP_IDF_TESTING
TEST_CASE("thread tests", "[thread]")
#else
void test_thread()
#endif
{
    RUN_TEST(test_thread_get_id);
    RUN_TEST(test_lock_guard);
#ifdef ESTD_OS_FREERTOS
    test_freertos();
#endif
}

#else
void test_thread()
{
    
}
#endif