#include <catch2/catch.hpp>

#include <estd/bit.h>

TEST_CASE("bit operations")
{
    SECTION("byteswap")
    {
        SECTION("uint16_t")
        {
            auto v = estd::byteswap<uint16_t>(0x1234);

            REQUIRE(v == 0x3412);
        }
        SECTION("int32_t")
        {
            auto v = estd::byteswap<int32_t>(0x1234);

            REQUIRE(v == 0x34120000);
        }
    }
}