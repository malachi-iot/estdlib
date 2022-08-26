#include "unit-test.h"

#include <estd/thread.h>
#include <estd/mutex.h>
#include <estd/semaphore.h>

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

static estd::freertos::wrapper::semaphore sync_sem;
static int counter;

template <bool is_static>
static void freertos_mutex_looper(estd::freertos::mutex<is_static>& m, int* counter, bool increment)
{
    for(int i = 0; i < 50; ++i)
    {
        m.lock();

        if(increment)
            ++(*counter);
        else
            --(*counter);

        m.unlock();
    }
}

template <bool is_static>
static void test_freertos_mutex_task(void* p)
{
    sync_sem.take(portMAX_DELAY);   // signal that we have started

    freertos_mutex_looper(
        * (estd::freertos::mutex<is_static>*) p,
        &counter,
        true);

    sync_sem.give();                // signal that we have finished

    vTaskDelete(NULL);
}

// FIX: Incomplete unit test
template <bool is_static>
static void test_freertos_mutex_iteration(estd::freertos::mutex<is_static>& m)
{
    TEST_ASSERT_NOT_NULL(m.native_handle());

    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    counter = 0;

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
    
    sync_sem.give();    // wait till test_freertos_mutex starts
    
    freertos_mutex_looper(m, &counter, false);

    sync_sem.take(portMAX_DELAY);   // wait till test_freertos_mutex ends

    TEST_ASSERT_EQUAL(0, counter);
}

template <bool is_static>
static void test_freertos_mutex_iteration(estd::freertos::recursive_mutex<is_static>& m)
{
    TEST_ASSERT_NOT_NULL(m.native_handle());
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

static void test_freertos_recursive_mutex()
{
    {
        estd::freertos::recursive_mutex<true> m;

        test_freertos_mutex_iteration(m);
    }

    {
        estd::freertos::recursive_mutex<false> m;

        test_freertos_mutex_iteration(m);
    }
}


static void test_freertos_semaphore()
{
    {
        estd::freertos::counting_semaphore<4, true> s(0);
    }

    {
        estd::freertos::counting_semaphore<4, false> s(0);
    }
}

static void test_freertos_thread()
{

}


static void test_freertos()
{
    sync_sem.create_binary();

    RUN_TEST(test_freertos_mutex);
    RUN_TEST(test_freertos_mutex_static);
    RUN_TEST(test_freertos_recursive_mutex);
    RUN_TEST(test_freertos_semaphore);
    RUN_TEST(test_freertos_thread);

    sync_sem.free();
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