#include <catch.hpp>

#include "estd/vector.h"
#include "estd/array.h"
#include "mem.h"

using namespace estd;

TEST_CASE("array/vector tests")
{
    SECTION("Array")
    {
        array<int, 5> array1;
        int i = 0;

        for(; i < 5; i++)
            array1[i] = i;

        i = 0;

        for(auto i2 : array1)
        {
            REQUIRE(i2 == i);
            i++;
        }
    }
    SECTION("Array layer2")
    {
        int buf[5];
        layer2::array<int, 5> array1(buf);

        int i = 0;

        for(; i < 5; i++)
            array1[i] = i;

        i = 0;

        for(auto i2 : array1)
        {
            REQUIRE(i2 == i);
            i++;
        }
    }
    SECTION("Array layer3")
    {
        int buf[5];
        layer3::array<int> array1(buf);

        int i = 0;

        for(; i < 5; i++)
            array1[i] = i;

        i = 0;

        for(auto i2 : array1)
        {
            REQUIRE(i2 == i);
            i++;
        }
    }
}
