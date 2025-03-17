#include <catch2/catch.hpp>

#include <estd/ostream.h>
#include <estd/sstream.h>
#include <estd/iomanip.h>

using namespace estd;

#include "macro/push.h"

struct exp_manipulator : detail::ostream_functor_tag
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
    layer1::ostringstream<64> out;
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
        SECTION("int (hex)")
        {
            out.setf(ios_base::uppercase);
            out << hex;

            const auto& s = out.rdbuf()->str();

            SECTION("literal")
            {
                out << 12;

                REQUIRE(s[0] == 'C');
            }
            SECTION("uint8_t")
            {
                out << setfill('0');
                out << setw(2);

                uint8_t v = 0xF;

                out << v;

                REQUIRE(s == "0F");
            }
            SECTION("unsigned")
            {
                out << setfill('0');
                out << setw(2);

                unsigned v = 0xF;

                out << v;

                REQUIRE(s == "0F");

                out << ':' << v;

                // "The width property of the stream will be reset to zero (meaning "unspecified") if any of the following functions are called:"
                // ... "Overloads of basic_ostream::operator<<() that take arithmetic type"
                // https://en.cppreference.com/w/cpp/io/manip/setw#Notes
                REQUIRE(s == "0F:F");
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
    SECTION("non standard")
    {
        SECTION("underlying clear")
        {
            out << 20;

            REQUIRE(out_s == "20");

            out.rdbuf()->clear();

            REQUIRE(out_s.empty());

            out << 321;

            REQUIRE(out_s == "321");
        }
    }
    SECTION("streambuf reference")
    {
        using streambuf_type = layer1::basic_out_stringbuf<char, 256, true>;
        streambuf_type streambuf;
        detail::basic_ostream<streambuf_type&> out_ref(streambuf);

        out_ref << 2;
        out_ref << "Hello";
        out_ref << exp_manipulator(5);

        REQUIRE(streambuf.str() == "2Hello20");
    }
    SECTION("iomanipulators")
    {
        SECTION("setbase")
        {
            out << setbase(8);
            out << 8;

            REQUIRE(out_s == "10");
        }
    }
    SECTION("ospanstream")
    {
        char buf[128];
        estd::span<char> span(buf);

        estd::detail::ospanstream out(span);

        out << "hi2u";

        REQUIRE(out.tellp() == 4);
    }

}

#include "macro/pop.h"
