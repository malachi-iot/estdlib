#include <catch2/catch_all.hpp>

#include <estd/internal/platform.h>

// Brute force to decay-to-C mode, since we expect issues to present there
// more often than C++ wrapper mode
#undef FEATURE_STD_CMATH
#define FEATURE_STD_CMATH 0

#include <estd/cmath.h>

TEST_CASE("cmath")
{
    SECTION("round")
    {
        REQUIRE(estd::round(0.2) == 0.0);
    }
    SECTION("sin")
    {
        float v = 0;

        auto v2 = estd::sin(v);

        static_assert(std::is_floating_point<decltype(v2)>::value, "");

        REQUIRE(v2 == 0);
    }
}