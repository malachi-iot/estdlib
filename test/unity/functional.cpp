#include "unit-test.h"

#if defined (FEATURE_CPP_VARIADIC)
#include <estd/functional.h>

static void test_function_base()
{
    int counter = 0;

    estd::experimental::function_base<void()> fb;

    auto m = fb.make_inline([&](){++counter;});

    m._exec();

    TEST_ASSERT_EQUAL(1, counter);
}

static void test_inline_function()
{
    int counter = 0;

    auto _f = [&](){++counter;};
    estd::experimental::inline_function<decltype(_f), void()> _if(std::move(_f));

    _if();

    // testing copy/move operator/constructor
    auto _if2 = _if;

    TEST_ASSERT_EQUAL(1, counter);

    _if2();

    TEST_ASSERT_EQUAL(2, counter);

    // FIX: Something about the way we wrap up things causes issues.  RVO bug?
    auto f = estd::experimental::function<void()>::make_inline2([&](){++counter;});

    // PIO ESP32 This fails for 2/3 conditions.  For:
    // concept_fnptr1 - hard crash
    // concept_fnptr2 - does not appear to execute
    // concept_virtual - works OK
    // esp-idf ESP32 we get a hard error (upgraded from a warning) -
    // this call is using uninitialized '_f'
    f();

    TEST_ASSERT_EQUAL(3, counter);
}
#else
static void test_inline_function()
{
    
}
#endif

#ifdef ESP_IDF_TESTING
TEST_CASE("runtime function tests", "[functional]")
#else
void test_functional()
#endif
{
    RUN_TEST(test_function_base);
	RUN_TEST(test_inline_function);
}

