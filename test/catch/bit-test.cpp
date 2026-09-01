#include <catch2/catch_all.hpp>

#include <estd/bit.h>
#include <estd/internal/functional/hash.h>  // Just for access to fnv_modes

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

            auto v1 = estd::byteswap<int32_t>(-500);

            REQUIRE(estd::byteswap(v1) == -500);
        }
    }
    SECTION("bit packed")
    {
        // Special internal estd bit pack assist
        uint8_t data[4] {};

        using namespace estd::internal;

        SECTION("basic")
        {
            bit_packed_write<2, 2>(data, 3);
            REQUIRE(data[0] == 0xC);
            unsigned v = bit_packed_read<2, 2>(data);
            REQUIRE(v == 3);
        }
        SECTION("bit_packed")
        {
            using type = bit_packed<10, 2>;

            type::write(data, FNV_1A);
            REQUIRE(type::read(data) == FNV_1A);

            REQUIRE(data[0] == 0);
            REQUIRE(data[1] == 4);

            type::write(data, FNV_1);

            REQUIRE(data[1] == 0);
        }
    }
}
