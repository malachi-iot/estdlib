#include "unit-test.h"

/**
 * @file
 * Testing inbuilt compiler features platform by platform.  Not actually estdlib specific
 */

#if defined(FEATURE_CPP_TYPEOF)
static int test_typeof_value;

// typeof is GCC predecessor for decltype
// Unfortunately Blackfin only supports this in C not C++ mode
void test_typeof_keyword()
{
    typeof(test_typeof_value) value;
}
#endif

// more gcc testing
static void test_overflows()
{
    int math_result = 0;
    bool result;

    result = __builtin_add_overflow(10, 8, &math_result);

    TEST_ASSERT(result == false);
    TEST_ASSERT_EQUAL_INT(18, math_result);

    result = __builtin_mul_overflow(4, 10, &math_result);

    TEST_ASSERT(result == false);
    TEST_ASSERT_EQUAL_INT(40, math_result);
}

static void test_gcc_cpp()
{
#if defined(FEATURE_CPP_TYPEOF)
    RUN_TEST(test_typeof_keyword);
#endif
    RUN_TEST(test_overflows);
}



#ifdef ESP_IDF_TESTING
TEST_CASE("C++ language/compiler tests", "[cpp]")
#else
void test_cpp()
#endif
{
    test_gcc_cpp();
}