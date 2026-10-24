#include "unit-test.h"
#include "test-data.h"

#include <estd/flags.h>
#include <estd/internal/locale/ctype.h>

struct test1
{
    enum flags : uint8_t
    {
        none = 0,
        opt1 = 0x01,
        opt2 = 0x02,
        opt3 = 0x04
    };
};

ESTD_FLAGS(test1::flags)

static void test_flags_integration()
{
    //using ctype = estd::ctype<char>;

    //TEST_ASSERT_TRUE(ctype::is(ctype::alpha, 'c'));
}

static void test_flags_ll()
{
    test1::flags flags{};

    TEST_ASSERT_FALSE(flags & test1::opt1);

    //flags |= test1::opt1;
}


#ifdef ESP_IDF_TESTING
TEST_CASE("flags", "[flags]")
#else
void test_flags()
#endif
{
    RUN_TEST(test_flags_integration);
}

