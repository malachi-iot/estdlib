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

#include <estd/port/freertos/timer.h>
#include <estd/port/freertos/event_groups.h>

namespace freertos {

static estd::freertos::wrapper::semaphore sync_sem;
static estd::freertos::wrapper::task unity_task;
static int counter;

namespace notifications {

CONSTEXPR uint32_t
    started = 0x0001,
    finished = 0x0002;

}

static TaskHandle_t create_task(TaskFunction_t taskCode,
    const char* name,
    void* parameters)
{
    TaskHandle_t handle;

    BaseType_t xReturned = xTaskCreate(
        taskCode,
        name,
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

template <bool is_static, bool direct_task_notification>
static void test_mutex_task(void* p)
{
    // signal that we have started
    if(direct_task_notification)
        unity_task.notify(notifications::started, eSetBits);
    else
        sync_sem.take(portMAX_DELAY);

    mutex_looper(
        * (estd::freertos::mutex<is_static>*) p,
        &counter,
        true);

    sync_sem.give();                // signal that we have finished

    vTaskDelete(NULL);
}

template <bool direct_task_notification, bool is_static>
static void test_mutex_iteration(estd::freertos::mutex<is_static>& m)
{
    BaseType_t r;
    // DEBT: Use chrono to calculate 1s
    CONSTEXPR TickType_t ticksToWait = 100; // 1s, usually
    TEST_ASSERT_NOT_NULL(m.native_handle());

    counter = 0;

    m.lock();
    TEST_ASSERT_TRUE(m.unlock());

    estd::freertos::wrapper::task t = 
        create_task(test_mutex_task<is_static, direct_task_notification>,
        "test_mutex", &m);

    if(direct_task_notification)
    {
        uint32_t notifiedValue;
        r = xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, ticksToWait);
        TEST_ASSERT_TRUE(r);
        TEST_ASSERT_EQUAL(notifications::started, notifiedValue);
    }
    else
    {
        t.name();           // Just to quiet down unused 't' warnings
        sync_sem.give();    // wait till test_mutex starts
    }
    
    mutex_looper(m, &counter, false);

    r = sync_sem.take(ticksToWait);   // wait till test_mutex ends

    TEST_ASSERT_EQUAL(0, counter);
}

template <bool is_static>
static void test_mutex_iteration(estd::freertos::recursive_mutex<is_static>& m)
{
    TEST_ASSERT_NOT_NULL(m.native_handle());
}

static void test_semaphote_task(void* p)
{
    sync_sem.take(portMAX_DELAY);   // signal that we have started

    // signal that we have finished
    ((estd::freertos::internal::semaphore*) p)->release();

    vTaskDelete(NULL);
}


static void test_semaphore_iteration(estd::freertos::internal::semaphore& s, int max)
{
    TEST_ASSERT_NOT_NULL(s.native_handle());

    s.release();
    TEST_ASSERT_EQUAL(1, s.native_handle().count());
    s.acquire();
    
    if(max > 1)
    {
        // Not quite ready
        //TEST_ASSERT_TRUE(s.try_acquire_for(estd::chrono::milliseconds(100)));
    }

    create_task(test_semaphote_task, "test_semaphore", &s);

    sync_sem.give();    // wait till test_semaphote starts

    s.acquire();        // wait till test_semaphore ends

    TEST_ASSERT_EQUAL(0, s.native_handle().count());
}


static void test_mutex()
{
    // Dynamic (regular) mutex
    {
        estd::freertos::mutex<false> m;

        test_mutex_iteration<true>(m);
        test_mutex_iteration<false>(m);
    }
}

static void test_mutex_static()
{
    // Static mutex
    {
        estd::freertos::mutex<true> m;

        test_mutex_iteration<true>(m);
        test_mutex_iteration<false>(m);
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
        
        test_semaphore_iteration(s, s.max());
    }

    {
        estd::freertos::counting_semaphore<4, false> s(0);

        TEST_ASSERT_EQUAL(4, s.max());

        test_semaphore_iteration(s, s.max());
    }

    {
        estd::freertos::binary_semaphore<true> s;

        test_semaphore_iteration(s, s.max());
    }

    {
        estd::freertos::binary_semaphore<false> s;

        test_semaphore_iteration(s, s.max());
    }
}

static void test_thread()
{

}

static void timer_callback(TimerHandle_t xTimer)
{
    void* arg = pvTimerGetTimerID(xTimer);
    estd::freertos::internal::semaphore* s = (estd::freertos::internal::semaphore*) arg;

    s->release();
}

static void test_timer()
{
    // FIX: Really we should be passing in a 0 here to match std API signature.  FreeRTOS
    // doesn't give us an API for that though we do an initial take easily enough
    // NOTE: Making static just incase timer_callback takes way too long to do its thing
    static estd::freertos::binary_semaphore<true> sync;

    estd::freertos::timer<false> timer("unit test only",
        estd::chrono::milliseconds(100), false,
        &sync, &timer_callback);

    timer.native().start(0);

    bool result = sync.try_acquire_for(estd::chrono::milliseconds(200));

    TEST_ASSERT_TRUE(result);
}

static void event_group_task(void* arg)
{
    auto& e = * (estd::freertos::wrapper::event_group*) arg;

    e.set_bits(1);

    vTaskDelete(NULL);
}



static void test_event_groups()
{
    estd::freertos::event_group<true> e;
    create_task(event_group_task, "event group task", &e);

    TEST_ASSERT_TRUE(e.wait_bits(1, false, true, estd::chrono::milliseconds(100)));
}

}

static void test_freertos()
{
    freertos::sync_sem.create_binary();
    freertos::unity_task = estd::freertos::wrapper::task::current();

    RUN_TEST(freertos::test_mutex);
    RUN_TEST(freertos::test_mutex_static);
    RUN_TEST(freertos::test_recursive_mutex);
    RUN_TEST(freertos::test_semaphore);
    RUN_TEST(freertos::test_thread);
    RUN_TEST(freertos::test_timer);
    RUN_TEST(freertos::test_event_groups);

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