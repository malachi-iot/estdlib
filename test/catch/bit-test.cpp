#include <catch2/catch.hpp>

#include <estd/bit.h>

TEST_CASE("bit operations")
{
    SECTION("byteswap")
    {
        auto v = estd::byteswap<uint16_t>(0x1234);

        REQUIRE(v == 0x3412);
    }
}