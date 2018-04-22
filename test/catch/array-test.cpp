#include <catch.hpp>

#include "estd/vector.h"
#include "estd/array.h"
#include "mem.h"

TEST_CASE("array/vector tests")
{
    SECTION("basic vector")
    {
        estd::vector<int, _allocator> v;

        v.reserve(20);

        auto test = v[3];
        auto& test2 = test.lock();

        test2 = 5;

        REQUIRE(v[3].lock() == 5);
    }
    SECTION("basic vector 2")
    {
        estd::vector<int, _allocator> v;

        v.push_back(5);

        REQUIRE(v.size() == 1);
        REQUIRE(v.capacity() > 5);
        REQUIRE(v[0].lock() == 5);
    }
}
