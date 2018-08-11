//
// Created by malachi on 8/10/18.
//

#include <catch.hpp>

#include <estd/istream.h>
#include <estd/ostream.h>

TEST_CASE("iostreams")
{
    SECTION("A")
    {
        //estd::basic_streambuf<char> streambuf(*stdout);
        estd::ostream _cout(*stdout);

        _cout << "Got here" << estd::endl;
    }
}