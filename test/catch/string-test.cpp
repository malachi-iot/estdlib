#include <estd/string.h>
#include <estd/string_view.h>
#include <estd/vector.h>
#include <estd/charconv.h>
#include <cstdlib>

#include "mem.h"

using namespace estd;
//using namespace estd::experimental;


#include <catch.hpp>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

constexpr const char* test_str = "hello";
constexpr const char* test_str2 = "hi2u";


TEST_CASE("string tests")
{
    SECTION("string tests")
    {
        basic_string<char, std::char_traits<char>, _allocator<char>> test;

        test += test_str;

        REQUIRE(test == test_str);
    }
    SECTION("A")
    {
        //string val;
        basic_string<char, std::char_traits<char>, _allocator<char>> val;

        char buf[128];

        val += test_str;

        val.copy(buf, 128);

        REQUIRE(buf[0] == 'h');
        REQUIRE(buf[1] == 'e');
        REQUIRE(buf[2] == 'l');
        REQUIRE(buf[3] == 'l');
        REQUIRE(buf[4] == 'o');
    }
    SECTION("layer 1")
    {
        SECTION("string literal assignment")
        {
            layer1::string<32> s = "hi2u";
        }
    }
    SECTION("layer 1 null terminated")
    {
        // FIX: Does not appear to properly initialize
        // underlying buffer with a leading null-termination
        // but just happens to accidentally work
        layer1::basic_string<char, 20> s;
        int sz = sizeof(s);
        int len = s.length();

        REQUIRE(len == 0);

        s += test_str;

        REQUIRE(s.length() == 5);
        REQUIRE(s == test_str);
    }
    SECTION("layer 1 size-tracked")
    {
        layer1::string<20, false> s;

        int sz = sizeof(s);

        REQUIRE(s.length() == 0);

        s += test_str;

        REQUIRE(s.length() == 5);
        REQUIRE(s == test_str);
    }
    SECTION("layer 2")
    {
        SECTION("null terminated")
        {
            SECTION("empty string")
            {
                char buf[128] = ""; // as per spec, this actually
                layer2::basic_string<char, 20> s(buf, 0);
                //layer2::basic_string<const char, 20> s2 = "hi";

                int sz = sizeof(s);

                REQUIRE(s.length() == 0);

                s += test_str;

                REQUIRE(s.length() == 5);
                REQUIRE(s == test_str);

                // TODO: Doesn't work, but should
                // (make it interact with data() so that this *only* works with non-locking
                //  stuff)
                //layer2::string<> str3 = test_str;
            }
            SECTION("alternate initialization")
            {
                char buf[128] = ""; // as per spec, this actually
                layer2::basic_string<char, 20> s(buf, true);

                int sz = sizeof(s);

                REQUIRE(s.length() == 0);

                s += test_str;

                REQUIRE(s.length() == 5);
                REQUIRE(s == test_str);
            }
            SECTION("literal")
            {
                char buf[128];
                layer2::basic_string<const char, 4> str("val");

                str.copy(buf, 128);

                REQUIRE(buf[0] == 'v');
                REQUIRE(buf[1] == 'a');

                REQUIRE(str.size() == 3);
                REQUIRE(str.max_size() == 3);

                layer2::basic_string<const char, 0> str2 = str;

                REQUIRE(str2.size() == 3);
                REQUIRE(str == str2);
                REQUIRE(str == "val");

                // NOTE: Doesn't work by design - loses const-qualifiers
                //layer2::string<> str3 = "hi2u";
            }
        }
        SECTION("assignment to literal")
        {
            char buf[128];
            layer2::string<> str(buf, 0);

            // Remember, an = here is a copy into while an =
            // during instantiation is specifying the underlying layer2 ptr
            str = "hi2u";

            REQUIRE(str == "hi2u");
            REQUIRE(str.size() == 4);
            REQUIRE(strcmp(buf, "hi2u") == 0);
            REQUIRE(str.data() == buf);
        }
        SECTION("absolute length")
        {
            char s[128];

            strcpy(s, test_str2);

            layer2::string<4, false> str(s);

            auto size = str.size();
            auto length = str.length();

            // FIX: size comes out to be 0, which is wrong
            // Needs to use 'dynamic_array_length<TAllocator, false, true>' but instead is using
            // 'dynamic_array_length<TAllocator, false, false>'
            //REQUIRE(size == 4);
            //REQUIRE(length == 4);
        }
        SECTION("make_string (experimental)")
        {
            char buf[128];
            auto str = layer2::experimental::make_string(buf, 0);

            str = "hi2u";

            REQUIRE(str == "hi2u");
            REQUIRE(str.size() == 4);
            REQUIRE(strcmp(buf, "hi2u") == 0);
            REQUIRE(str.data() == buf);
        }
    }
    SECTION("layer 3 null terminated")
    {
        char buf[128];
        char buf2[128] = "val";
        layer3::basic_string<char> str(buf2, 3);

        str.copy(buf, 128);

        REQUIRE(buf[0] == 'v');
        REQUIRE(buf[1] == 'a');

        REQUIRE(str.size() == 3);
        REQUIRE(str.max_size() == 127);

        // FIX: For C++98 compat, we still have to associate the 'parity' version of things
        // and since we don't, there's a redundant max-sizing in here of 0
        str += ":hi2u";

        REQUIRE(str.size() == 8);

        REQUIRE(buf2[3] == ':');
        REQUIRE(buf2[7] == 'u');

    }
    SECTION("dynamic_array low level allocator test")
    {
        //estd::vector<uint8_t, test_t> d;
        estd::vector<uint8_t, internal::single_fixedbuf_allocator<uint8_t, 10>> d;

        d.push_back(3);

        uint8_t v = d[0];
        auto vv = (int)v;

        // flaky sometimes, likely due to underlying locking magic
        REQUIRE(v == 3);
        REQUIRE(vv == 3);
    }
    SECTION("layer1 / single_fixedbuf_allocator")
    {
        estd::basic_string<char, std::char_traits<char>,
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
                layer1::allocator<char, 30>
#else
                internal::single_fixedbuf_allocator<char, 30>
#endif
                > s;
        char buf[128];

        s += test_str;

        buf[s.copy(buf, 128)] = 0;

        REQUIRE(strcmp(buf, test_str) == 0);
    }
    SECTION("single_nullterm_fixedbuf_allocator")
    {
        estd::basic_string<
                char,
                std::char_traits<char>,
                internal::single_fixedbuf_allocator<char, 30>,
                experimental::null_terminated_string_policy<std::char_traits<char> >
                > s;
        int sz = sizeof(s);
        char buf[128];

        REQUIRE(s.size() == 0);

        s += test_str;

        REQUIRE(s == test_str);

        REQUIRE(s.size() == 5);

        buf[s.copy(buf, 128)] = 0;

        REQUIRE(strcmp(buf, test_str) == 0);
    }
    // see 'layer 2 null terminated' section for comments
    SECTION("layer2 -> layer3 promotion")
    {
        layer2::basic_string<const char, 10> val = test_str;
        layer3::basic_string<const char> val2 = val;
        layer1::string<128> val3;

        int sz1 = sizeof(val);
        int sz2 = sizeof(val2);

        REQUIRE(sz1 == sizeof(char*));
        REQUIRE(sz2 == sizeof(char*) + sizeof(size_t));

        char buf[128];

        buf[val2.copy(buf, 128)] = 0;

        val3 = val;

        REQUIRE(val3 == test_str);
        REQUIRE(val3 == buf);
    }
    SECTION("layer3 null terminated")
    {
        char buf[128];

        layer3::basic_string<char> s(buf, 0);

        REQUIRE(s.size() == 0);

        s += test_str;

        REQUIRE(s.size() == 5);
        REQUIRE(s == test_str);
    }
    SECTION("layer3 length-specified (not null terminated)")
    {
        char buf[128];

        memset(buf, 0, sizeof(buf));

        layer3::basic_string<char, false> s(buf, 0);

        REQUIRE(s.size() == 0);

        s += test_str;

        REQUIRE(s.size() == 5);
        REQUIRE(s == test_str);

        // since buf was zeroed out beforehand, we can treat it
        // as a C-style string
        layer3::basic_string<char, false> s2(buf);

        REQUIRE(s == s2);

        layer1::string<20> s3(s2);

        REQUIRE(s3 == s);
    }
    SECTION("Non experimental conversion between layers")
    {
        char buf2[100];
        char buf3[100];

        layer1::basic_string<char, 100> s1;
        layer2::basic_string<char, 100> s2(buf2, 0);

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

        layer2::string<> s2(buf2, 0);
        layer3::string s3(buf3, 0);

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

        layer2::basic_string<const char, 0> s2 = "Hello World";
        layer2::const_string s3 = "Hello World, again";

        char buf[128];

        buf[s2.copy(buf, sizeof(buf))] = 0;

        REQUIRE(strlen(buf) > 0);
        REQUIRE(s2 == "Hello World");
        REQUIRE(s3 == "Hello World, again");
    }
    SECTION("Erase portion of string")
    {
        layer1::string<40> s = "Hello World";

        s.erase(1, 1);

        REQUIRE(s == "Hllo World");

        s.pop_back();

        REQUIRE(s == "Hllo Worl");
    }
    SECTION("String iteration")
    {
        layer1::string<40> s = "Hello World";

        uint8_t i = 0;

        for(auto c : s)
        {
            INFO("i = " << i);
            REQUIRE(s[i++] == c);
        }
    }
    SECTION("layer3 copy constructor")
    {
        layer2::basic_string<const char, 0> s = "Hello, World";
        layer3::basic_string<const char> s2 = s;

        // works, though underlying buffer_size is max-value
        // cause of N=0 above.  Tolerable, but I'd prefer it
        // to deduce the size from strlen in this case
        REQUIRE(s2 == "Hello, World");
    }
    SECTION("layer3 const compare")
    {
        layer3::const_string s("test");

        INFO("Evaluating" << s);
        REQUIRE(s.length() == 4);

        // because we've denoted basic_string as not null terminated.  But, because it's const,
        // we'll never get to use that extra byte
        REQUIRE(s.capacity() == 4);
        REQUIRE(s.max_size() == 4);

        REQUIRE(s == "test");

        SECTION("compare against layer1")
        {
            layer1::string<20> s2 = "test";

            REQUIRE(s == s2);
        }
    }
    SECTION("to_string")
    {
        char buf[128];
        layer2::string<> s(buf, 0);

        REQUIRE(s.size() == 0);

        s += "The value is ";

        SECTION("int")
        {
            s += to_string(123);

            REQUIRE(s == "The value is 123");
        }
        SECTION("unsigned")
        {
            unsigned v = 123;

            s += to_string(v);

            REQUIRE(s == "The value is 123");
        }
        SECTION("int16_t")
        {
            int16_t v = -1000;

            s += to_string(v);

            REQUIRE(s == "The value is -1000");
        }
        SECTION("unsigned long long")
        {
            uint64_t v = 999999999999999;

            s += to_string(v);

            REQUIRE(s == "The value is 999999999999999");
        }
        /* Can't yet do because internal::maxStringLength<int128_t> is
         * slightly tricky
        SECTION("unsigned long long")
        {
            unsigned long long v = 999999999999999;

            s += to_string(v);

            REQUIRE(s == "The value is 999999999999999");
        }*/
    }
    SECTION("starts_with")
    {
        layer3::const_string s("test");

        REQUIRE(s.starts_with("te"));
        REQUIRE(!s.starts_with("st"));
    }
    SECTION("string_view")
    {
        string_view sv("test", 4);
        string_view sv2 = sv;
        string_view sv3 = "test3";

        int sz = sizeof(sv);

        // policy makes all strings default to size type of uint16_t
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        REQUIRE(sizeof(string_view::size_type) == sizeof(uint16_t));
#endif
        REQUIRE(sz == sizeof(char*) + sizeof(size_t));

        REQUIRE(sv3.starts_with(sv));
        REQUIRE(sv2 == sv);
        REQUIRE(sv2.compare(sv) == 0);
        REQUIRE(sv2 == "test");

        // NOTE: Only works in an experimental capacity right now.  Adjusted
        // layer3::allocator to have experimental setters to adjust its pointer
        // and size
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        sv3.remove_suffix(2);

        REQUIRE(sv3 == "tes");

        sv3.remove_prefix(1);

        REQUIRE(sv3 == "es");
#endif

        // Does not compile, as is correct behavior - string_views are read only except
        // for remove_suffix and remove_prefix
        //sv3 += "T";

        SECTION("from layer1::string")
        {
            layer1::string<32> s = "hi2u";

            estd::string_view v = s;

            REQUIRE(v == "hi2u");
        }
        SECTION("from layer2::string")
        {
            layer2::const_string s = "hi2u";

            estd::string_view v = s;

            REQUIRE(v == "hi2u");
        }

        sv3 = "test3";

        estd::string_view sv4 = sv3.substr(1, 2);

        const char* helper = sv4.lock();

        REQUIRE(sv4 == "es");
    }
    SECTION("conversion")
    {
        SECTION("stol")
        {
            estd::layer2::const_string val = "1234";
            long value = estd::stol(val);

            REQUIRE(value == 1234);

            estd::layer2::const_string val2 = "4321";
            std::size_t len;
            value = estd::stol(val2, &len);

            REQUIRE(value == 4321);
            REQUIRE(len == 4);
        }
        SECTION("stoul")
        {
            std::size_t len;
            estd::layer2::const_string val = "1234";
            long value = estd::stoul(val, &len);

            REQUIRE(value == 1234);
            REQUIRE(len == 4);
        }
        SECTION("stoi")
        {
            std::size_t len;
            estd::layer2::const_string val = "-1234";
            int value = estd::stoi(val, &len);

            REQUIRE(value == -1234);
            REQUIRE(len == 5);
        }
        SECTION("stoi [unsupported non-null-terminated]")
        {
            // FIX:
            // layer2::string ends up using a length_helper_t of type dynamic_array_length
            // which is a runtime length tracker rather than compile time.  And,
            // not surprisingly, it doesn't have length 4 but rather value of 0 -
            // and that might be accidental/undefined.
            estd::layer2::string<4, false> val = "1234";
            /*
            char buf[128];
            val.copy(buf, sizeof(buf));
            estd::layer2::string<> val2(buf);
            int value = estd::stoi(val2); */
        }
    }
    SECTION("errc")
    {
        // TODO: Move this error eval code elsewhere
        estd::errc error(estd::errc::invalid_argument);

        REQUIRE(error == errc::invalid_argument);
        REQUIRE(error != errc::result_out_of_range);

        REQUIRE(error == EINVAL);
    }
    SECTION("internal")
    {
        using namespace estd::internal::legacy;

        SECTION("maxStringLength (legacy)")
        {
            REQUIRE(maxStringLength<uint8_t>() == 3);
            REQUIRE(maxStringLength<int8_t>() == 4);
            REQUIRE(maxStringLength<float>() == 32);

            // Unsupported == 0
            REQUIRE(maxStringLength<estd::layer2::const_string>() == 0);
        }
    }
}

#pragma GCC diagnostic pop