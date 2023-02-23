#include <catch.hpp>

#include <estd/system_error.h>

#include <estd/expected.h>

TEST_CASE("expected")
{
    SECTION("int value_type")
    {
        SECTION("default")
        {

        }
        SECTION("specific value initialized")
        {
            estd::expected<int, estd::errc> e(10);

            REQUIRE(*e == 10);
        }
    }
    SECTION("void value_type")
    {
        estd::expected<void, estd::errc> e;
    }
}