#include <catch.hpp>

#include <estd/type_traits.h>

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
}
