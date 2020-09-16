#include "unit-test.h"

#include <estd/type_traits.h>

using namespace estd;

// TODO: VisualDSP does have some sort of _Alignas keyword, but it seems to differ from
// https://en.cppreference.com/w/c/language/_Alignas

static void test_align_1()
{
    typedef typename aligned_storage<sizeof(TestStruct), __alignof__(TestStruct)>::type data_type;

    data_type data;

    TEST_ASSERT_EQUAL_INT(sizeof(int), sizeof(data_type));
}

#ifdef ESP_IDF_TESTING
TEST_CASE("alignment tests", "[align]")
#else
void test_align()
#endif
{
    RUN_TEST(test_align_1);
}
