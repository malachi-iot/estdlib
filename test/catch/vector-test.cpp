#include <catch.hpp>

#include <estd/vector.h>

#include "mem.h"

using namespace estd;


TEST_CASE("vector tests")
{
    SECTION("basic vector")
    {
        estd::vector<int, _allocator<int>> v;

        v.reserve(20);

        auto test = v[3];
        auto& test2 = test.lock();

        test2 = 5;

        REQUIRE(v[3].lock() == 5);
    }
    SECTION("basic vector 2")
    {
        estd::vector<int, _allocator<int>> v;

        v.push_back(5);

        REQUIRE(v.size() == 1);
        REQUIRE(v.capacity() > 5);
        REQUIRE(v[0].lock() == 5);
    }
    SECTION("Vector iterator")
    {
        estd::vector<int, _allocator<int>> v;

        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        auto i = v.begin();

        i++;

        REQUIRE(i.lock() == 2);

        i.unlock();

        i++;
        i++;

        REQUIRE(i == v.end());
    }
    SECTION("Vector insert")
    {
        estd::vector<int, _allocator<int>> v;

        v.push_back(1);
        v.push_back(3);
        v.push_back(4);

        auto i = v.begin();

        i++;

        v.insert(i, 2);

        int counter = 0;

        for(auto i2 : v)
        {
            counter++;
            INFO("Counter: " << counter);
            REQUIRE(counter == *i2);
        }

    }
}
