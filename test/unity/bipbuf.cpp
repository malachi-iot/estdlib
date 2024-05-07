#include <estd/internal/bip/buffer.h>

#include "unit-test.h"

static void test_bipbuf_0()
{
    estd::layer1::bipbuf<0> buf;

    TEST_ASSERT_EQUAL(sizeof(bipbuf_t), sizeof(buf));
}

static void test_bipbuf_1()
{
    estd::layer1::bipbuf<64> buf;

    TEST_ASSERT_EQUAL(64, buf.unused());

    buf.offer((const unsigned char*)"hello", 5);

    TEST_ASSERT_EQUAL(59, buf.unused());
    auto peeked = (char*) buf.peek(5);
    TEST_ASSERT_NOT_NULL(peeked);
    TEST_ASSERT_EQUAL('h', *peeked);
}


#ifdef ESP_IDF_TESTING
TEST_CASE("bipbuffer tests", "[bipbuf]")
#else
void test_bipbuf()
#endif
{
    RUN_TEST(test_bipbuf_0);
    RUN_TEST(test_bipbuf_1);
}