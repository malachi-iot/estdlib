#include <catch.hpp>

#include <estd/ostream.h>
#include <estd/sstream.h>
#include <estd/iomanip.h>

using namespace estd;

TEST_CASE("ostream")
{
    experimental::ostringstream<64> out;

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
    SECTION("formatting")
    {
        out << setw(5);

        SECTION("fill")
        {
            out << setfill('0');

            out << 'H';

            auto s = out.rdbuf()->str();

            REQUIRE(s.length() == 6);

            REQUIRE(s[0] == '0');
        }
        SECTION("setw")
        {
            out << 'H';

            auto s = out.rdbuf()->str();

            REQUIRE(s.length() == 6);
        }
    }
}