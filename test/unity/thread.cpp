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

namespace freertos {

static estd::freertos::wrapper::semaphore sync_sem;
static int counter;

static TaskHandle_t create_task(TaskFunction_t taskCode,
    const char* name,
    void* parameters)
{
    TaskHandle_t handle;

    BaseType_t xReturned = xTaskCreate(
        taskCode,
        "test_mutex",
        2048,
        parameters,
        4,      // DEBT: Need a more clear or configurable priority
        &handle);

    TEST_ASSERT_EQUAL(pdPASS, xReturned);

    // Since we abort if fail, we can return handle in a
    // more basic way
    return handle;
}

template <bool is_static>
static void mutex_looper(estd::freertos::mutex<is_static>& m, int* counter, bool increment)
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
static void test_mutex_task(void* p)
{
    sync_sem.take(portMAX_DELAY);   // signal that we have started

    mutex_looper(
        * (estd::freertos::mutex<is_static>*) p,
        &counter,
        true);

    sync_sem.give();                // signal that we have finished

    vTaskDelete(NULL);
}

// FIX: Incomplete unit test
template <bool is_static>
static void test_mutex_iteration(estd::freertos::mutex<is_static>& m)
{
    TEST_ASSERT_NOT_NULL(m.native_handle());

    counter = 0;

    m.lock();
    TEST_ASSERT_TRUE(m.unlock());

    create_task(test_mutex_task<is_static>, "test_mutex", &m);
    
    sync_sem.give();    // wait till test_mutex starts
    
    mutex_looper(m, &counter, false);

    sync_sem.take(portMAX_DELAY);   // wait till test_mutex ends

    TEST_ASSERT_EQUAL(0, counter);
}

template <bool is_static>
static void test_mutex_iteration(estd::freertos::recursive_mutex<is_static>& m)
{
    TEST_ASSERT_NOT_NULL(m.native_handle());
}

static void semaphore_tester(estd::freertos::internal::semaphore& s)
{
    s.release();
    s.acquire();
}

static void test_semaphote_task(void* p)
{
    sync_sem.take(portMAX_DELAY);   // signal that we have started

    semaphore_tester(* (estd::freertos::internal::semaphore*) p);

    sync_sem.give();                // signal that we have finished

    vTaskDelete(NULL);
}


static void test_semaphore_iteration(estd::freertos::internal::semaphore& s)
{
    TEST_ASSERT_NOT_NULL(s.native_handle());

    s.release();
    TEST_ASSERT_EQUAL(1, s.native_handle().count());
    s.acquire();

    create_task(test_semaphote_task, "test_semaphore", &s);

    sync_sem.give();    // wait till test_semaphote starts

    sync_sem.take(portMAX_DELAY);

    TEST_ASSERT_EQUAL(0, s.native_handle().count());
}


static void test_mutex()
{
    // Dynamic (regular) mutex
    {
        estd::freertos::mutex<false> m;

        test_mutex_iteration(m);
    }

    // FIX: The binary flavor causes a lockup
    return;

    // Dynamic (regular) mutex, binary
    {
        estd::freertos::mutex<false> m(true);

        test_mutex_iteration(m);
    }
}

static void test_mutex_static()
{
    // Static mutex
    {
        estd::freertos::mutex<true> m;

        test_mutex_iteration(m);
    }

    return;

    // Static mutex, binary
    {
        estd::freertos::mutex<true> m(true);

        test_mutex_iteration(m);
    }
}

static void test_recursive_mutex()
{
    {
        estd::freertos::recursive_mutex<true> m;

        test_mutex_iteration(m);
    }

    {
        estd::freertos::recursive_mutex<false> m;

        test_mutex_iteration(m);
    }
}


static void test_semaphore()
{
    {
        estd::freertos::counting_semaphore<4, true> s(0);

        TEST_ASSERT_EQUAL(4, s.max());
        
        test_semaphore_iteration(s);
    }

    {
        estd::freertos::counting_semaphore<4, false> s(0);

        TEST_ASSERT_EQUAL(4, s.max());

        test_semaphore_iteration(s);
    }

    {
        estd::freertos::binary_semaphore<true> s;

        test_semaphore_iteration(s);
    }

    {
        estd::freertos::binary_semaphore<false> s;

        test_semaphore_iteration(s);
    }
}

static void test_thread()
{

}

}

static void test_freertos()
{
    freertos::sync_sem.create_binary();

    RUN_TEST(freertos::test_mutex);
    RUN_TEST(freertos::test_mutex_static);
    RUN_TEST(freertos::test_recursive_mutex);
    RUN_TEST(freertos::test_semaphore);
    RUN_TEST(freertos::test_thread);

    freertos::sync_sem.free();
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