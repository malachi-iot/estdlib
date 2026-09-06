#include <catch2/catch_all.hpp>

#include <estd/sstream.h>

using namespace estd;

#include "macro/push.h"

namespace {

constexpr char test_str1[] = "Hello";
constexpr char test_str2[] = "10:20\n";

}

TEST_CASE("istringstream")
{
    char buf[128]{};

    SECTION("layer2")
    {
        SECTION("null terminated")
        {
            layer2::istringstream<> in(test_str1);

            in >> buf;

            REQUIRE(string_view(buf) == test_str1);
        }
        SECTION("sized")
        {
            layer2::istringstream<sizeof(test_str1), false> in(test_str1);

            in >> buf;

            REQUIRE(string_view(buf) == test_str1);
        }
    }
    SECTION("layer3")
    {
        // Unhappy, presumably due to debt of:
        // https://github.com/malachi-iot/estdlib/issues/88
        // https://github.com/malachi-iot/estdlib/issues/127
        //layer3::basic_istringstream<const char> in(test_str1);

        //in >> test_str1;
    }
}

//template <class Streambuf>
template <class Impl>
void test_out(detail::basic_ostream<detail::streambuf<Impl>>& out)
{
    using string_type = typename Impl::string_type;
    const string_type& str = out.rdbuf()->str();

    out << test_str1;

    REQUIRE(str == test_str1);

    // NOTE: Doesn't work, because stringstream is very append-centric
    //out.seekp(0, ios_base::beg);
    // 'str' is always const, so can't do this either
    //str.clear();
    out.rdbuf()->clear();
    out << hex << 0x10 << ':' << 0x20 << endl;

    REQUIRE(str == test_str2);
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
        SECTION("seek")
        {
            layer1::ostringstream<128> out;

            // out_stringbuf doesn't actually handle seeking.  Eventually, perhaps
            out << "Hello";
            out.seekp(0);
            REQUIRE(out.fail());
            //out << 'J';
            //REQUIRE(out_s == "Jello");
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
            layer3::basic_ostringstream<char, true> out(buf);

            test_out(out);
        }
        SECTION("sized")
        {
            layer3::basic_ostringstream<char, false> out(buf);

            test_out(out);
        }
    }
}

#include "macro/pop.h"
