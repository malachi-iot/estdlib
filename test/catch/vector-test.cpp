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
    SECTION("layer1 vector")
    {
        estd::layer1::vector<int, 10> v;

        REQUIRE(v.size() == 0);

        v.push_back(3);

        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 3);
    }
    SECTION("layer2 vector")
    {
        // Not quite ready, need to de-const the dynamic_array init chain
        /*
        int buf[10];
        estd::layer2::vector<int, 10> v(buf);

        REQUIRE(v.size() == 0);

        v.push_back(3);

        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 3); */
    }
    SECTION("vector with semi-complex type/find")
    {
        struct complex
        {
            int value;
            const char* s;
        };

        estd::layer1::vector<complex, 10> v;

        //std::find_if(v.begin(), v.end(), [](const complex& value) { return value.value == 3; });

    }
    SECTION("vector with actual default std::allocator")
    {
        // eventually will be phased out to use our estd::allocator always
        estd::vector<int> v;

        //v.push_back(3);
    }
}
