#include <estd/string.h>
#include <estd/vector.h>
#include <cstdlib>
#include <ostream>

#include "mem.h"

using namespace estd;
//using namespace estd::experimental;

namespace std
{

template <class TChar, class TCharTraits, class TAllocator>
std::ostream& operator <<( std::ostream& os, estd::basic_string<TChar, TCharTraits, TAllocator> const& value)
{
    int len = value.size();
    /*
    TChar* s = value.lock();
    // because we can't be sure of null termination without doing more checks

    while(len--) os << *s++;

    value.unlock(); */

    char buf[256];
    char* s = buf;

    value.copy(buf, sizeof(buf));

    while(len--) os << *s++;

    return os;
}

}



#include <catch.hpp>


TEST_CASE("string tests")
{
    SECTION("string tests")
    {
        basic_string<char, std::char_traits<char>, _allocator<char>> test;

        test += "hello";
    }
    SECTION("A")
    {
        //string val;
        basic_string<char, std::char_traits<char>, _allocator<char>> val;

        char buf[128];

        val += "hello";

        val.copy(buf, 128);

        REQUIRE(buf[0] == 'h');
        REQUIRE(buf[1] == 'e');
        REQUIRE(buf[2] == 'l');
        REQUIRE(buf[3] == 'l');
        REQUIRE(buf[4] == 'o');
    }
    SECTION("layer 1 null terminated")
    {
        layer1::basic_string<char, 20> s;
        int sz = sizeof(s);

        REQUIRE(s.length() == 0);

        s += "hello";

        REQUIRE(s.length() == 5);
        REQUIRE(s == "hello");
    }
    SECTION("layer 2 (non-experimental) null terimnated")
    {
        char buf[128];
        layer2::basic_string<char, 20> s(buf);
        //layer2::basic_string<const char, 20> s2 = "hi";

        int sz = sizeof(s);

        REQUIRE(s.length() == 0);

        s += "hello";

        REQUIRE(s.length() == 5);
        REQUIRE(s == "hello");
    }
#ifdef UNUSED
    SECTION("layer 2 null terminated")
    {
        char buf[128];
        experimental::layer2::basic_string<char, 4> str("val");

        str.copy(buf, 128);

        REQUIRE(buf[0] == 'v');
        REQUIRE(buf[1] == 'a');

        REQUIRE(str.size() == 3);
        REQUIRE(str.max_size() == 3);

        // only viable in C++11 right now, due to N == 0 behavior
        // probably should make a distinct base class 'unbounded' basic_string
        // but I kind of like the conveinience of N == 0 behavior
        experimental::layer2::basic_string<char> str2 = str;

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
#endif
    SECTION("dynamic_array low level allocator test")
    {
        //estd::vector<uint8_t, test_t> d;
        estd::vector<uint8_t, internal::single_fixedbuf_allocator<uint8_t, 10>> d;

        d.push_back(3);

        auto& v = d[0];
        auto vv = (int)v;

        // flaky sometimes, likely due to underlying locking magic
        REQUIRE(v == 3);
        REQUIRE(vv == 3);
    }
    SECTION("single_fixedbuf_allocator")
    {
        estd::basic_string<char, std::char_traits<char>, internal::single_fixedbuf_allocator<char, 30>> s;
        char buf[128];

        s += "hello";

        buf[s.copy(buf, 128)] = 0;

        REQUIRE(strcmp(buf, "hello") == 0);
    }
    SECTION("single_nullterm_fixedbuf_allocator")
    {
        estd::basic_string<char, std::char_traits<char>, internal::single_fixedbuf_allocator<char, 30, true>> s;
        int sz = sizeof(s);
        char buf[128];

        REQUIRE(s.size() == 0);

        s += "hello";

        REQUIRE(s == "hello");

        REQUIRE(s.size() == 5);

        buf[s.copy(buf, 128)] = 0;

        REQUIRE(strcmp(buf, "hello") == 0);
    }
#ifdef UNUSED
    SECTION("layer2 -> layer3 promotion")
    {
        experimental::layer2::basic_string<char, 10> val = "hello";
        experimental::layer3::basic_string<char> val2 = val;

        int sz1 = sizeof(val);
        int sz2 = sizeof(val2);

        char buf[128];

        buf[val2.copy(buf, 128)] = 0;
    }
#endif
    SECTION("Non-experimental layer3")
    {
        char buf[128];

        layer3::basic_string<char> s(buf);

        REQUIRE(s.size() == 0);

        s += "hello";

        REQUIRE(s.size() == 5);
        REQUIRE(s == "hello");
    }
    SECTION("Non experimental conversion between layers")
    {
        char buf2[100];
        char buf3[100];

        layer1::basic_string<char, 100> s1;
        layer2::basic_string<char, 100> s2(buf2);

        s1 = "Hello";

        REQUIRE(s1 == "Hello");

        s2 = s1;

        REQUIRE(s2 == "Hello");
        REQUIRE(s2 == s1);
    }
    SECTION("Aliases/typedefs for layers")
    {
        char buf2[100];
        char buf3[100];

        layer1::string<100> s1;

        // This invokes some kind of infinite loop program never ends
        // Filters down into map_base somehow... ??
        s1 = "Hello";

        layer2::string<> s2(buf2);
        layer3::string s3(buf3);

        s2 = s1;

        REQUIRE(s1 == "Hello");
        REQUIRE(s2 == "Hello"); // Fails due to string<> def above, but shouldn't (should grow unboundedly)

        s1 += s2;

        // this works if we get by s2 == hello test
        REQUIRE(s1 == "HelloHello");

        s3 = s1;

        REQUIRE(s3 == s1);

        s1 = s2;

        REQUIRE(s1 == "Hello");
    }
    SECTION("indexer: layer1")
    {
        layer1::string<20> s = "hi2u";

        REQUIRE(s == "hi2u");

        s[2] = '3';

        REQUIRE(s == "hi3u");
    }
    SECTION("String literal assignment")
    {
        // DOES work sort of, but many operations like .copy don't work right
        // because it is too const'd up
        const layer2::basic_string<const char, 0> s = "Hello World";

        // specializations keep const char* from being 'initialized' with a leading null termination
        layer2::basic_string<char, 0, true, std::char_traits<char>, const char*> s2 = "Hello World";
        layer2::const_string s3 = "Hello World, again";

        char buf[128];

        buf[s2.copy(buf, sizeof(buf))] = 0;

        REQUIRE(strlen(buf) > 0);
        REQUIRE(s2 == "Hello World");
        REQUIRE(s3 == "Hello World, again");

        /*
        estd::basic_string<
                        char, std::char_traits<char>,
                        estd::experimental::single_fixedbuf_allocator < char, 0, true, char* > > s2 = "hello"; */
    }
    SECTION("Erase portion of string")
    {
        layer1::string<40> s = "Hello World";

        s.erase(1, 1);

        REQUIRE(s == "Hllo World");
    }
}
