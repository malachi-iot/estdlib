#include <catch2/catch_all.hpp>

#include <estd/cstdlib.h>

TEST_CASE("cstdlib")
{
    SECTION("abs")
    {
        int v = -1;
        v = estd::abs(v);

        REQUIRE(v == 1);
    }
}