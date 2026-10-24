#include <catch2/catch_all.hpp>

#include <estd/flags.h>

using namespace estd;

enum synthetic_flags    // NOLINT
{
    SN_NONE = 0x00,
    SN_CONST = 0x01,
    SN_DOUBLE = 0x02
};

enum class scoped_flags
{
    none = 0,
    f1 = 0x01,
    f2 = 0x02,
};

ESTD_FLAGS(synthetic_flags)

template <synthetic_flags, class = void>
struct test1
{
    using value_type = int;
};


template <synthetic_flags f>
struct test1<f, enable_if_t<is_set(f & SN_DOUBLE)>>
{
    static constexpr synthetic_flags flags = f;
    static constexpr bool is_const = f & SN_CONST;
    using value_type = double;
};

template <synthetic_flags f>
void assert_double(test1<f>)
{
    // C++14 is sensitive to non-constexpr linking, so test that too
    if(!is_set(test1<f>::flags & SN_DOUBLE))
    {
        CATCH_ERROR("Shouldn't get here");
    }
}


static constexpr synthetic_flags test_flags_constexpr(synthetic_flags f)
{
    synthetic_flags f2{};

    f2 |= f;

    return f2;
}

static_assert(test_flags_constexpr(SN_CONST) == SN_CONST, "constexpr compound assignment test fails");
static_assert(SN_CONST == detail::flags<synthetic_flags>(SN_CONST), "");
static_assert(detail::flags<synthetic_flags>(SN_CONST) == SN_CONST, "");
static_assert(detail::flags<scoped_flags>(scoped_flags::f1) == scoped_flags::f1, "");
static_assert(scoped_flags::f1 == detail::flags<scoped_flags>(scoped_flags::f1), "");


TEST_CASE("flags (compile-time capable)", "[flags]")
{
    using t1 = test1<SN_NONE>;
    using t2 = test1<SN_DOUBLE>;
    using t3 = test1<SN_DOUBLE | SN_CONST>;

    static_assert(std::is_same<t1::value_type, int>::value, "");    // NOLINT
    static_assert(std::is_same<t2::value_type, double>::value, ""); // NOLINT
    static_assert(t3::is_const, "");    // NOLINT

    assert_double(t2{});

    detail::flags<synthetic_flags> s1{0};

    s1 = SN_DOUBLE;

    REQUIRE(s1 & SN_DOUBLE);
}
