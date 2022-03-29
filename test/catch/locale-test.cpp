#include <catch.hpp>

#include <estd/algorithm.h>
#include <estd/locale.h>

using namespace estd;

TEST_CASE("locale")
{
    experimental::locale l;

    SECTION("isspace")
    {
        REQUIRE(experimental::isspace(' ', l));
    }
    SECTION("num_get")
    {
        experimental::num_get<char, const char*> n;
        ios_base::iostate state;
        ios_base holder;
        long v;

        SECTION("simple source")
        {
            const char* in = "123";

            n.get(in, in + 4, state, holder, v);

            REQUIRE(v == 123);
        }
        SECTION("complex iterator")
        {
            const char* in = "123/";

            // TBD
        }
    }
    SECTION("use_facet")
    {
        SECTION("ctype")
        {
            constexpr char c = 'a';
            char result = experimental::use_facet<experimental::ctype<char> >(l).widen(c);
            REQUIRE(result == c);
        }
    }
}