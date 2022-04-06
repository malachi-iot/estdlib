#include <catch.hpp>

#include <estd/iterator.h>
#include <estd/sstream.h>

TEST_CASE("iterator")
{
    SECTION("istreambuf_iterator")
    {
        // FIX: layer3::stringbuf has a compilation error in/around 'size_type'
        //estd::layer3::stringbuf in;

        SECTION("end-of-stream")
        {
            estd::experimental::istreambuf_iterator<estd::layer2::stringbuf> it;
        }
        SECTION("from istream")
        {
            
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