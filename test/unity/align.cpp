#include "unit-test.h"

#include <estd/type_traits.h>
#include <estd/array.h>

using namespace estd;

// VisualDSP has no alignas or _Alignas keyword

static void test_align_1()
{
    typedef typename aligned_storage<sizeof(TestStruct), __alignof__(TestStruct)>::type data_type;

    TEST_ASSERT_EQUAL_INT(sizeof(int), sizeof(data_type));
}

static void test_align_2()
{
    experimental::aligned_storage_array<TestStruct, 10> a;
}

#ifdef ESP_IDF_TESTING
TEST_CASE("alignment tests", "[align]")
#else
void test_align()
#endif
{
    RUN_TEST(test_align_1);
}
