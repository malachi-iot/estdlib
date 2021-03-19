#include <catch.hpp>

#include <estd/locale.h>

using namespace estd;

TEST_CASE("locale")
{
    experimental::locale l;

    SECTION("isspace")
    {
        REQUIRE(experimental::isspace(' ', l));
    }
    SECTION("use_facet")
    {

    }
}