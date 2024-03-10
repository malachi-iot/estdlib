#include <catch.hpp>

#include <estd/sstream.h>
#include <estd/iostream.h>

TEST_CASE("iostreams")
{
    estd::layer1::stringstream<256> ss;
    using streambuf_type = estd::layer1::basic_stringbuf<char, 256, false>;
    using ios_type = estd::detail::basic_iostream<streambuf_type>;
    ios_type ios;
    estd::layer1::string<128> s;

    ios << "Going";
    ios >> s;

    REQUIRE(s == "Going");
}