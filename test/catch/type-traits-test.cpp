#include <catch.hpp>

#include <estd/type_traits.h>
#include <estd/limits.h>

using namespace estd;


TEST_CASE("type traits tests")
{
    SECTION("make_signed")
    {
        typedef make_signed<uint16_t>::type should_be_int16_t;

        REQUIRE(sizeof(should_be_int16_t) == sizeof(int16_t));
        REQUIRE(is_signed<should_be_int16_t>::value);
        REQUIRE(!is_signed<uint16_t>::value);
    }
    SECTION("decay")
    {
        int digits = numeric_limits<decay_t<uint8_t> >::digits;
        REQUIRE(digits == 8);
        digits = numeric_limits<decay_t<int16_t> >::digits;
        REQUIRE(digits == 15);
        digits = numeric_limits<decay_t<uint16_t> >::digits;
        REQUIRE(digits == 16);
    }
}
