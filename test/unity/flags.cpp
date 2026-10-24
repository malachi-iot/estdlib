#include "unit-test.h"
#include "test-data.h"

#include <estd/flags.h>
#include <estd/internal/locale/ctype.h>
#include <estd/locale.h>

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

struct test_scoped
{
    enum class flags : uint8_t
    {
        none = 0,
        opt1 = 0x01,
        opt2 = 0x02,
        opt3 = 0x04
    };
};

ESTD_FLAGS(test1::flags)
ESTD_FLAGS(test_scoped::flags)

static void test_flags_integration()
{
    using namespace estd;

    // DEBT: Use dtype directly
    locale::type<locale::iso::en_US,
        locale::encodings::UTF8> l;
    //using ctype = estd::ctype<char>;
    auto f = use_facet<ctype<char>>(l);

    //TEST_ASSERT_TRUE(ctype::is(ctype::alpha, 'c'));
    TEST_ASSERT_TRUE(f.is(ctype_base::lower, 'c'));
}

static void test_flags_ll()
{
    test1::flags flags{};
    test_scoped::flags scoped{};

    TEST_ASSERT_FALSE(flags & test1::opt1);
    TEST_ASSERT_FALSE(scoped & test_scoped::flags::opt1);

    flags |= test1::opt1;
    scoped |= test_scoped::flags::opt1;

    TEST_ASSERT_TRUE(flags & test1::opt1);
    TEST_ASSERT_TRUE(scoped & test_scoped::flags::opt1);
}


#ifdef ESP_IDF_TESTING
TEST_CASE("flags", "[flags]")
#else
void test_flags()
#endif
{
    RUN_TEST(test_flags_integration);
    RUN_TEST(test_flags_ll);
}

