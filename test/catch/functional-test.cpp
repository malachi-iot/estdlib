#include <catch.hpp>

#include <estd/functional.h>


void do_something(const char* msg)
{
    printf("I did something: %s\n", msg);
}


TEST_CASE("functional")
{
    SECTION("tuple")
    {
        auto tuple = estd::make_tuple(0, 1.0, 2);

        int sz = sizeof(tuple);

        REQUIRE(sz == sizeof(long) * 2 + sizeof(double));

        auto val1 = estd::get<0>(tuple);

        REQUIRE(val1 == 0);

        auto val2 = estd::get<1>(tuple);

        REQUIRE(val2 == 1.0);
    }
    SECTION("A")
    {
        auto b = estd::bind(&do_something, "hello");

        //b();
    }
}
