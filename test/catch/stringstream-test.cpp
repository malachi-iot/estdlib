#include <catch2/catch_all.hpp>

#include <estd/sstream.h>

using namespace estd;

#include "macro/push.h"

TEST_CASE("istringstream")
{

}

template <class Impl, class Impl2>
void test_out(detail::basic_ostream<Impl>& out,
    const detail::basic_string<Impl2>& wrapped)
{
    out << "Hello";

    REQUIRE(wrapped == "Hello");

    // NOTE: Doesn't work, because stringstream is very append-centric
    out.seekp(0, ios_base::beg);
    out << hex << 0x10 << ':' << 0x20;

    //REQUIRE(wrapped == "10:20");
}

TEST_CASE("ostringstream")
{
    SECTION("layer1")
    {

    }
    SECTION("layer2")
    {
        char buf[128]{};

        SECTION("explicit, null term")
        {
            layer2::string<> wrapped(buf);
            layer2::ostringstream<128> out(buf);

            test_out(out, wrapped);
        }
        SECTION("implicit, null term")
        {
            layer2::string<> wrapped(buf);
            layer2::ostringstream<> out(buf);

            test_out(out, wrapped);
        }
        SECTION("explicit, sized")
        {
            layer2::string<128, false> wrapped(buf);
            layer2::ostringstream<128, false> out(buf);

            // FIX:
            //test_out(out, wrapped);
        }
    }
    SECTION("layer3")
    {
        char buf[128]{};

        SECTION("null term")
        {
            layer3::string wrapped(buf);
            layer3::ostringstream<true> out(buf);

            test_out(out, wrapped);
        }
        SECTION("sized")
        {
            layer3::basic_string<char, false> wrapped(buf);
            layer3::ostringstream<false> out(buf);

            // FIX:
            //test_out(out, wrapped);
        }
    }
}

#include "macro/pop.h"
