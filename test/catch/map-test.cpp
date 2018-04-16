#include <catch.hpp>

#include <estd/map.h>

using namespace estd;

TEST_CASE("map-test")
{
    SECTION("layer 1 map")
    {
        layer1::map<int, int, 4> map;

        const int* result = map[3];

        REQUIRE(result == NULLPTR);
    }
    SECTION("layer 2 map")
    {
        layer2::map<int, int, 4>::value_type buf[4];

        layer2::map<int, int, 4> map(buf);

        const int* result = map[3];

        REQUIRE(result == NULLPTR);
    }
}
