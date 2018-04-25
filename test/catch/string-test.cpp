#include <catch.hpp>

#include <estd/string.h>
#include <estd/vector.h>

#include "mem.h"

using namespace estd;

template <class T>
using test_t = experimental::single_fixedbuf_allocator<T, 10>;



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

        // only viable in C++11 right now, due to N == 0 behavior
        // probably should make a distinct base class 'unbounded' basic_string
        // but I kind of like the conveinience of N == 0 behavior
        experimental::layer2::basic_string<const char> str2 = str;

        REQUIRE(str2.size() == 3);
        REQUIRE(str == str2);
        REQUIRE(str == "val");

        //experimental::layer2::string<> str3 = str;
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
    SECTION("dynamic_array low level allocator test")
    {
        estd::vector<uint8_t, test_t> d;

        d.push_back(3);

        //REQUIRE(d[0] == 3);
    }
}
