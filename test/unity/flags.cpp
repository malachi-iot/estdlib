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

// 24OCT26 MB DEBT: Temporarily living here while we sort this feature out
// (flags doesn't seem to fully support |= like this yet)
#define ESTD_FLAGS_DUAL_OP(Enum, op, op2)    \
ESTD_CPP_CONSTEXPR(14) Enum& operator op (Enum& lhs, Enum rhs)   \
{\
    using flags = estd::detail::flags<Enum>;\
    return lhs = static_cast<Enum>(lhs op2 rhs);\
}

ESTD_FLAGS_DUAL_OP(test1::flags, |=, |)
ESTD_FLAGS_DUAL_OP(test1::flags, &=, &)
ESTD_FLAGS_DUAL_OP(test_scoped::flags, |=, |)
ESTD_FLAGS_DUAL_OP(test_scoped::flags, &=, &)

/*
inline test1::flags& operator |=(estd::detail::flags<test1::flags>& lhs, test1::flags rhs)
{
    return lhs |= rhs;
} */

#if __cplusplus >= 201402L
static constexpr void test_flags_constexpr()
{
    // TBD
}
#endif

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

