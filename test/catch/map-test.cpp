#include <catch.hpp>

#include <estd/map.h>
#include <estd/string.h>

#include "mem.h"

using namespace estd;

TEST_CASE("map-test")
{
    SECTION("layer 1 map")
    {
        typedef layer1::map<int, int, 4> map_t;
        estd::array<map_t::value_type, 4> buf;

#ifdef FEATURE_ESTD_LEGACY_ARRAY
        buf[0].first = 0;
        buf[1].first = 0;
        buf[2].first = 0;
        buf[3].first = 0;

        map_t map(buf);

        const int* result = map[3];

        REQUIRE(result == NULLPTR);
#endif
    }
    SECTION("layer 1 map init list")
    {
        layer1::map<int, int, 4> map =
        {
            { 1, 77 },
            { 2, 78 },
            { 3, 79 },
            { 4, 80 }
        };

        auto result = map[2];

        REQUIRE(*result == 78);
    }
    SECTION("layer 2 map")
    {
        layer2::map<int, int, 4>::value_type buf[4] =
        {
            { 1, 77 },
            { 2, 78 },
            { 4, 79 },
            { 5, 80 }
        };

        layer2::map<int, int, 4> map(buf);

        const int* result = map[3];

        REQUIRE(result == NULLPTR);
    }
    SECTION("layer 3 map")
    {
        layer3::map<int, int>::value_type buf[] =
        {
            { 1, 77 },
            { 2, 78 },
            { 3, 79 },
            { 4, 80 }
        };

        layer3::map<int, int> map(buf);

        const int* result = map[3];

        REQUIRE(result != NULLPTR);
        REQUIRE(*result == 79);
    }
}
