#include <catch.hpp>

#include <estd/ostream.h>
#include <estd/sstream.h>
#include <estd/iomanip.h>

using namespace estd;

#include "macro/push.h"

struct exp_manipulator : internal::ostream_functor_tag
{
    int v;

    constexpr explicit exp_manipulator(int v) : v{v*2} {}

    template <class TStreambuf, class TBase>
    void operator()(estd::detail::basic_ostream<TStreambuf, TBase>& out) const
    {
        out << v * 2;
    }
};

TEST_CASE("ostream")
{
    experimental::ostringstream<64> out;
    const auto& out_s = out.rdbuf()->str();

    SECTION("output character")
    {
        out << 'A';

        REQUIRE(out.rdbuf()->str()[0] == 'A');
    }
    SECTION("output integer literal")
    {
        out << 7;

        REQUIRE(out.rdbuf()->str()[0] == '7');
    }
    SECTION("output integer variable")
    {
        int v = 7;

        out << v;

        REQUIRE(out.rdbuf()->str()[0] == '7');
    }
    SECTION("output double variable")
    {
        double v = 7;

        // FIX: c++ permits this narrowing conversion, but we want to prohibit it
        //out << v;

        // TODO: Comes out as a char, so raw 7 not '7' - see above
        //REQUIRE(out.rdbuf()->str()[0] == '7');
    }
    SECTION("output bool")
    {
        const bool v = true;

        // Not ready yet
        //out << v;

        //REQUIRE(out.rdbuf()->str() == "true");
    }
    SECTION("formatting")
    {
        SECTION("fill")
        {
            out << setw(5);
            out << setfill('0');

            SECTION("char")
            {
                out << 'H';

                auto s = out.rdbuf()->str();

                REQUIRE(s.length() == 5);

                REQUIRE(s[0] == '0');
            }
            SECTION("int")
            {
                out << 12;

                auto s = out.rdbuf()->str();

                REQUIRE(s[0] == '0');
            }
        }
        SECTION("setw")
        {
            out << setw(5);

            SECTION("char")
            {
                out << 'H';

                auto s = out.rdbuf()->str();

                REQUIRE(s.length() == 5);
            }
            SECTION("int")
            {
                out << 12;

                auto s = out.rdbuf()->str();

                REQUIRE(s.length() == 5);
                REQUIRE(s[0] == ' ');
            }
        }
        SECTION("clock style")
        {
            // TODO: Consider time facet stuff, but mate it more to steady_clock and friends rather than
            // time_t

            out << setfill('0');
            out << setw(2) << 1 << ':' << setw(2) << 30;

            auto s = out.rdbuf()->str();

            REQUIRE(s == "01:30");
        }
    }
    SECTION("experimental")
    {
        out << exp_manipulator(5);

        REQUIRE(out_s == "20");
    }
    SECTION("arrays/strings")
    {
        SECTION("layer1 string")
        {
            layer1::string<64> s{"hello"};

            out << s;

            REQUIRE(out_s == "hello");
        }
    }
}

#include "macro/pop.h"
