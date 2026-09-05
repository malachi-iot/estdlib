#include <catch2/catch_all.hpp>

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

// EXPERIMENTAL
template <class Derived>
class sstream_crtp : public Derived
{
    template <class T>
    struct helper
    {
        using streambuf_type = typename T::streambuf_type;
        using string_type = typename streambuf_type::string_type;
    };

public:
    const typename Derived::streambuf_type::string_type& str()
    {
        auto self = static_cast<Derived*>(this);

        return self->rdbuf()->str();
    }
};

using l1_test = sstream_crtp<layer1::ostringstream<64>>;

TEST_CASE("ostream")
{
    layer1::ostringstream<64> out;
    const layer1::string<64>& out_s = out.rdbuf()->str();

    SECTION("output character")
    {
        out << 'A';

        REQUIRE(out_s[0] == 'A');
    }
    SECTION("output integer literal")
    {
        out << 7;

        REQUIRE(out_s[0] == '7');
    }
    SECTION("output integer variable")
    {
        int v = 7;

        out << v;

        REQUIRE(out_s[0] == '7');
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

        out.flags(ios_base::boolalpha);

        out << v;

        REQUIRE(out.rdbuf()->str() == "true");
    }
#if FEATURE_STD_CHARCONV
    SECTION("output float")
    {
        float v = 7.1;

        out << v;

        // FIX: Precision appears one character too big.  However, sick mofo just sat down
        // next to me sniffling and sneezing & I don't need to get the plague so this
        // glitch will have to live on for now.
        REQUIRE(out.rdbuf()->str() == "7.100000");
    }
#endif
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
        SECTION("int (hex) + uppercase")
        {
            out << uppercase << hex;

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
            SECTION("clear uppercase")
            {
                out << nouppercase << 12;

                REQUIRE(s[0] == 'c');
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

        estd::ospanstream out(span);

        out << "hi2u";

        REQUIRE(out.tellp() == 4);
    }
    SECTION("crtp (EXPERIMENTAL)")
    {
        l1_test l1;

        l1 << "Hello";

        REQUIRE(l1.str() == "Hello");
    }
    SECTION("repositioning")
    {
        SECTION("span")
        {
            uint8_t buf[128];
            const uint8_t test1[] { 1, 2, 3, 4 };
            estd::span<uint8_t> span(buf);

            estd::detail::basic_ospanstream<uint8_t> out(span);

            out.write(test1, 4);
            out.seekp(0);
            out.put(4);

            REQUIRE(out.good());

            // DEBT: Use Catch2 proper array compares
            REQUIRE(buf[0] == 4);
            REQUIRE(buf[1] == test1[1]);

            out.seekp(-1, ios_base::cur);

            REQUIRE(out.good());

            out.put(5);

            REQUIRE(buf[0] == 5);

            out.seekp(1);
            out.put(6);

            REQUIRE(buf[1] == 6);
        }
    }
}

#include "macro/pop.h"
