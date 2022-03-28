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
        SECTION("ctype")
        {
            constexpr char c = 'a';
            char result = experimental::use_facet<experimental::ctype<char> >(l).widen(c);
            REQUIRE(result == c);
        }
        SECTION("get_num")
        {

        }
    }
}