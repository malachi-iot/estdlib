#include <catch.hpp>

#include <estd/iterator.h>
#include <estd/sstream.h>

TEST_CASE("iterator")
{
    SECTION("istreambuf_iterator")
    {
        // FIX: layer3::stringbuf is fiddly.  Needs:
        // - better const awareness
        // -- maybe a specialization replacing layer3::const_string into layer3::basic_string<const char>
        //estd::layer3::stringbuf in("hello");
        estd::layer3::stringbuf::size_type sz = 5;
        estd::layer3::stringbuf in((char*)"hello", 5, sz);

        SECTION("end-of-stream")
        {
            estd::experimental::istreambuf_iterator<estd::layer2::stringbuf> it;
        }
        SECTION("from istream")
        {
            estd::experimental::istreambuf_iterator<estd::layer3::stringbuf> it(&in);

            REQUIRE(*it++ == 'h');
            REQUIRE(*it++ == 'e');
            REQUIRE(*it++ == 'l');
            REQUIRE(*it == 'l');
            REQUIRE(*++it == 'o');
        }
    }
    SECTION("filter_iterator")
    {
        auto it = estd::experimental::make_filter_iterator(
            [](char c)
            {
                return c == '/' ? 0 : c;
            },
            "hi2u/");

        //estd::layer1::string<32>(it, 0);
        estd::layer1::string<32> s;

        while(*it != 0)
        {
            s += *it++;
        }

        REQUIRE(s == "hi2u");


        //estd::experimental::filter_iterator
    }
}