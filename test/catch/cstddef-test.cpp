#include <estd/cstddef.h>
#include <estd/cstdint.h>

#include <catch2/catch.hpp>

using namespace estd;

TEST_CASE("cstddef")
{
    SECTION("byte")
    {
        estd::byte val = to_byte(2);

        SECTION(">>=")
        {
            val >>= 1;

            REQUIRE(to_integer<int>(val) == 1);
        }
        SECTION("<<=")
        {
            val <<= 2;

            REQUIRE(to_integer<int>(val) == 8);
        }
        SECTION("|=")
        {
            estd::byte other_val = to_byte(4);

            val |= other_val;

            REQUIRE(to_integer<int>(val) == 6);
        }
    }
}