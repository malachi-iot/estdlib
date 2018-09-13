#include <catch.hpp>

#include <estd/tuple.h>

using namespace std;

TEST_CASE("tuple")
{
    SECTION("a")
    {
        tuple<int> t(5);

        REQUIRE(get<0>(t) == 5);
    }
}
