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

static void test_gcc_cpp()
{
    RUN_TEST(test_typeof_keyword);
}

#else
static void test_gcc_cpp()
{
}
#endif


void test_cpp()
{
    test_gcc_cpp();
}