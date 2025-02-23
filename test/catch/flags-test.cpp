#include <catch2/catch.hpp>

#include <estd/flags.h>

enum synthetic_flags
{
    SN_NONE = 0x00,
    SN_CONST = 0x01,
    SN_DOUBLE = 0x02
};

ESTD_FLAGS(synthetic_flags)

template <synthetic_flags, class = void>
struct test1
{
    using value_type = int;
};


template <synthetic_flags f>
struct test1<f, estd::enable_if_t<f & SN_DOUBLE>>
{
    static constexpr bool is_const = f & SN_CONST;
    using value_type = double;
};


TEST_CASE("flags (compile-time capable)", "[flags]")
{
    using t1 = test1<SN_NONE>;
    using t2 = test1<SN_DOUBLE>;
    using t3 = test1<SN_DOUBLE | SN_CONST>;

    static_assert(std::is_same<t1::value_type, int>::value, "");
    static_assert(std::is_same<t2::value_type, double>::value, "");
    static_assert(t3::is_const, "");
}
