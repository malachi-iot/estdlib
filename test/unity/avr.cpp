#include "unit-test.h"

#ifdef __AVR__
#include <estd/exp/pgm/string.h>

static const char test1[] PROGMEM = "Hello AVR:";
const char test2[] = "Hello AVR:";

static void test_pgm_string1()
{
    estd::pgm_string s(test1), _s(test1);
    estd::layer1::string<64> s2;

    // NOTE: Don't think there's a way to overload for PROGMEM
    //TEST_ASSERT_TRUE(s == test1);

    //TEST_ASSERT_TRUE(s == _s);

    const auto& alloc1 = s.get_allocator();
    const auto& alloc2 = _s.get_allocator();

    TEST_ASSERT_EQUAL(alloc1.data(), alloc2.data());
}


static void test_pgm_string2()
{
    estd::pgm_string s(test1);
    estd::layer1::string<64> s2;

    s2 += s;

    // TODO: no specialized 'equal' yet
    //TEST_ASSERT_TRUE(s == s2);
    //TEST_ASSERT_TRUE(s2 == s);

    TEST_ASSERT_TRUE(s == test2);
    TEST_ASSERT_TRUE(s2 == test2);
}

void test_avr_pgm()
{
    RUN_TEST(test_pgm_string1);
    RUN_TEST(test_pgm_string2);
}
#endif