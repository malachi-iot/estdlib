#include <catch.hpp>

#include <estd/tuple.h>

#include "test-data.h"

using namespace estd::test;

TEST_CASE("tuple")
{
    SECTION("std lib version")
    {
        using namespace std;

        tuple<int> t(5);

        REQUIRE(get<0>(t) == 5);
    }
    SECTION("estd lib version")
    {
        using namespace estd;

        tuple<int> t(5);

        REQUIRE(get<0>(t) == 5);
    }
    SECTION("value initialization")
    {
        using namespace estd;

        // As per https://en.cppreference.com/w/cpp/utility/tuple/tuple, tuples
        // value-initialize, which means default constructors shall run

        tuple<DefaultConstructor, int> t;

        const DefaultConstructor& dc = get<0>(t);

        REQUIRE(dc.val == DefaultConstructor::default_value());
    }
}
