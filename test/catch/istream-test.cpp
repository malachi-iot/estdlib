#include <catch.hpp>

#include <estd/istream.h>
#include <estd/sstream.h>

// DEBT: As we fiddle with bringing string_view istreambuf online, we must manually
// include this
#include <estd/string_view.h>

using namespace estd;

TEST_CASE("istream")
{
    typedef internal::impl::basic_stringbuf<layer1::string<32> > impl_type;
    using streambuf_type = detail::streambuf<impl_type>;

    SECTION("basics")
    {
        const layer1::string<32> str = "hi2u";

        detail::basic_istream<streambuf_type> _cin(str);

        SECTION("readsome")
        {
            char localbuf[128];
            estd::streamsize read_back = _cin.readsome(localbuf, str.length());

            REQUIRE(read_back == str.length());
            REQUIRE(localbuf[0] == str[0]);
        }
        SECTION("seekg")
        {
            _cin.seekg(1, estd::ios_base::cur);
            REQUIRE('i' == _cin.get());
        }
    }
    SECTION("parsing")
    {
        SECTION("integer conversion")
        {
            layer1::string<32> str{"2 xyz"};

            detail::basic_istream<streambuf_type> _cin(str);

            int val = - 1;

            _cin >> val;

            REQUIRE(_cin.good());
            REQUIRE(val == 2);

            _cin >> val;

            REQUIRE(val == 0);
            REQUIRE(_cin.good() == false);

            if(_cin) FAIL();
        }
        SECTION("integer overflow")
        {
            SECTION("signed")
            {
                layer1::string<32> str{"-70000"};

                detail::basic_istream<streambuf_type> _cin(str);

                int16_t v = 0;

                _cin >> v;

                REQUIRE(_cin.fail());
                REQUIRE(v == estd::numeric_limits<decltype(v)>::min());
            }
            SECTION("unsigned")
            {
                layer1::string<32> str{"2 70000"};
                detail::basic_istream<streambuf_type> _cin(str);

                uint16_t v = 0;

                _cin >> v;

                REQUIRE(_cin.eof() == false);
                REQUIRE(_cin.good());
                REQUIRE(v == 2);

                _cin >> v;

                REQUIRE(_cin.fail());
                REQUIRE(v == estd::numeric_limits<decltype(v)>::max());
            }
        }
        SECTION("istringstream")
        {
            layer1::istringstream<32> in = "123 456";

            int val;

            in >> val;

            REQUIRE(val == 123);

            in >> val;

            REQUIRE(val == 456);
        }
        SECTION("with const char")
        {
            const char* buf = "hi2u";

            SECTION("layer2 string")
            {
                using stream_type = layer2::basic_istringstream<const char>;
                stream_type in(buf);
                layer1::string<32> buf2;

                in >> buf2;

                REQUIRE(buf2 == buf);
            }
            SECTION("string view")
            {
                /* ALmost, but not quite.  Doesn't play nice with a 'Container'
                 * constraint
                 *
                using stream_type = detail::basic_istream<
                    estd::internal::impl::v0::basic_sviewbuf<char>>;
                stream_type in(buf);
                layer1::string<32> buf2;

                in >> buf2;

                REQUIRE(buf2 == buf);
                */
            }
        }
    }
    SECTION("cin")
    {
        // limited testing since an automated test shouldn't pause for input
        estd::istream _cin(*stdin);

        // POSIX in doesn't reveal in_avail
        //_cin.rdbuf()->in_avail();
    }
}
