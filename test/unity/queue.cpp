#include "unit-test.h"

#include <estd/queue.h>

#ifdef ESTD_OS_FREERTOS
#include <estd/port/freertos/queue.h>
#endif

using namespace estd;

CONSTEXPR int test_value_1 = 5;
CONSTEXPR int test_value_2 = 10;


static void test_queue_int()
{
    layer1::queue<int, 10> q;

    q.push(test_value_1);
    q.push(test_value_2);

    int value = q.front();

    TEST_ASSERT_EQUAL_INT(test_value_1, value);

    q.pop();
    q.pop();

    TEST_ASSERT_EQUAL_INT(0, q.size());
}


static void test_queue_struct()
{
    layer1::queue<TestStruct, 10> q;

    // NOTE: Interesting that VisualDSP permits this - taking a const reference to a temporary.  I'm glad
    // it does, but it is a potentially dangerous operation (fortunately not in this case)
    q.push(TestStruct(test_value_1));
}


static void test_priority_queue()
{
#if FEATURE_ESTD_PRIORITY_QUEUE
    layer1::priority_queue<int, 10> q;

    // FIX: Each 'push' consumes an additional 32 bytes of stack on esp-idf
    q.push(7);
    q.push(3);
    q.push(8);
    q.push(4);

    TEST_ASSERT_EQUAL_INT(8, q.top());
#endif
}

#ifdef ESTD_OS_FREERTOS

template <class T>
static void test_freertos_queue(freertos::internal::queue<T>& queue)
{
    const unsigned val = 7;
    unsigned val2 = 0;

    TEST_ASSERT_EQUAL(0, queue.messages_waiting());
    TEST_ASSERT_TRUE(queue.send(val, freertos::max_delay()));
    TEST_ASSERT_EQUAL(1, queue.messages_waiting());
    TEST_ASSERT_TRUE(queue.receive(&val2));
    TEST_ASSERT_EQUAL(val, val2);
}

static void test_freertos_queue()
{
    // mainly just wrappers
    {
        freertos::queue<unsigned, false> dynamic_q(10);
        test_freertos_queue(dynamic_q);
    }

#if configSUPPORT_STATIC_ALLOCATION
    {
        unsigned storage[10];
        freertos::queue<unsigned, true> static_q(storage, 10);
        test_freertos_queue(static_q);
    }

    {
        freertos::layer1::queue<unsigned, 10> static_q2;
        test_freertos_queue(static_q2);
    }
#endif
}
#endif

#ifdef ESP_IDF_TESTING
TEST_CASE("queue tests", "[queue]")
#else
void test_queue()
#endif
{
    RUN_TEST(test_queue_int);
    RUN_TEST(test_queue_struct);
    RUN_TEST(test_priority_queue);
#ifdef ESTD_OS_FREERTOS
    RUN_TEST(test_freertos_queue);
#endif
}
