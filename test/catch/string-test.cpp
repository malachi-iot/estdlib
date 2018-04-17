#include <catch.hpp>

#include <estd/string.h>

#include "mem.h"

using namespace estd;

TEST_CASE("string tests")
{
    SECTION("string tests")
    {
        basic_string<char, char_traits<char>, _allocator> test;

        test += "hello";
    }
    SECTION("A")
    {
        //string val;
        basic_string<char, char_traits<char>, _allocator> val;

        char buf[128];

        val += "hello";

        val.copy(buf, 128);

        REQUIRE(buf[0] == 'h');
        REQUIRE(buf[1] == 'e');
        REQUIRE(buf[2] == 'l');
        REQUIRE(buf[3] == 'l');
        REQUIRE(buf[4] == 'o');
    }
}