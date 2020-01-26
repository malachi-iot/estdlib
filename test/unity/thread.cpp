#include "unit-test.h"

#include <estd/thread.h>
#include <estd/mutex.h>

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


#ifdef ESP_IDF_TESTING
TEST_CASE("thread tests", "[thread]")
#else
void test_thread()
#endif
{
    RUN_TEST(test_thread_get_id);
    RUN_TEST(test_lock_guard);
}