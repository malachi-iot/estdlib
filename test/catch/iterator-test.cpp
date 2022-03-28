#include <catch.hpp>

#include <estd/iterator.h>
#include <estd/sstream.h>

TEST_CASE("iterator")
{
    SECTION("istreambuf_iterator")
    {
        // FIX: layer3::stringbuf has a compilation error in/around 'size_type'
        //estd::layer3::stringbuf in;
        estd::experimental::istreambuf_iterator<estd::layer2::stringbuf> it;
    }
}