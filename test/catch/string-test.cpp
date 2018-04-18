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
    SECTION("layer 2 null terminated")
    {
        char buf[128];
        experimental::layer2::basic_string<const char, 4> str("val");

        str.copy(buf, 128);

        REQUIRE(buf[0] == 'v');
        REQUIRE(buf[1] == 'a');

        REQUIRE(str.size() == 3);
        REQUIRE(str.max_size() == 3);
    }
    SECTION("layer 3 null terminated")
    {
        char buf[128];
        char buf2[128] = "val";
        experimental::layer3::basic_string<char> str(buf2);

        str.copy(buf, 128);

        REQUIRE(buf[0] == 'v');
        REQUIRE(buf[1] == 'a');

        REQUIRE(str.size() == 3);
        REQUIRE(str.max_size() == 127);

        str += ":hi2u";

        REQUIRE(str.size() == 8);

        REQUIRE(buf2[3] == ':');
        REQUIRE(buf2[7] == 'u');

    }
}
