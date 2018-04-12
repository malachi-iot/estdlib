#include <catch.hpp>

#include <estd/queue.h>

using namespace estd;

TEST_CASE("queue-test")
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
    SECTION("A")
    {

    }
}
