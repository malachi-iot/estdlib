#include <catch2/catch_all.hpp>

#include <estd/sstream.h>

using namespace estd;

#include "macro/push.h"

TEST_CASE("istringstream")
{

}

//template <class Streambuf>
template <class Impl>
void test_out(detail::basic_ostream<detail::streambuf<Impl>>& out)
{
    using string_type = typename Impl::string_type;
    const string_type& str = out.rdbuf()->str();

    out << "Hello";

    REQUIRE(str == "Hello");

    // NOTE: Doesn't work, because stringstream is very append-centric
    //out.seekp(0, ios_base::beg);
    // 'str' is always const, so can't do this either
    //str.clear();
    out.rdbuf()->clear();
    out << hex << 0x10 << ':' << 0x20 << endl;

    REQUIRE(str == "10:20\n");
}

TEST_CASE("ostringstream")
{
    SECTION("layer1")
    {
        SECTION("null term")
        {
            layer1::ostringstream<128> out;

            test_out(out);
        }
        SECTION("sized")
        {
            layer1::ostringstream<128, false> out;

            test_out(out);
        }
    }
    SECTION("layer2")
    {
        char buf[128]{};

        SECTION("explicit, null term")
        {
            layer2::ostringstream<128> out(buf);

            test_out(out);
        }
        SECTION("implicit, null term")
        {
            layer2::ostringstream<> out(buf);

            test_out(out);
        }
        SECTION("explicit, sized")
        {
            layer2::ostringstream<128, false> out(buf);

            test_out(out);
        }
    }
    SECTION("layer3")
    {
        char buf[128]{};

        SECTION("null term")
        {
            layer3::ostringstream<true> out(buf);

            test_out(out);
        }
        SECTION("sized")
        {
            layer3::ostringstream<false> out(buf);

            test_out(out);
        }
    }
}

#include "macro/pop.h"
