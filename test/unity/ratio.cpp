#include "unit-test.h"

#include <estd/ratio.h>

typedef estd::ratio<1, 100> ratio1;
typedef estd::ratio<1, 30> ratio2;

#ifdef FEATURE_CPP_ALIASTEMPLATE
typedef estd::ratio_divide<ratio1, ratio2> divided;
#else
typedef estd::detail::ratio_divide<ratio1, ratio2> divided;
#endif

static void test_ratio_divide()
{
    TEST_ASSERT_EQUAL_INT(3, divided::num);
    TEST_ASSERT_EQUAL_INT(10, divided::den);
}

#ifdef ESP_IDF_TESTING
TEST_CASE("ratio tests", "[ratio]")
#else
void test_ratio()
#endif
{
    RUN_TEST(test_ratio_divide);
}