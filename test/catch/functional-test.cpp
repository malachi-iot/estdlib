#include <catch.hpp>

#include <estd/functional.h>


void do_something(const char* msg)
{
    printf("I did something: %s\n", msg);
}


TEST_CASE("functional")
{
    SECTION("A")
    {
        auto b = estd::bind(&do_something, "hello");

        //b();
    }
}
