#include <catch.hpp>

#include <estd/istream.h>
#include <estd/sstream.h>

using namespace estd;

TEST_CASE("istream")
{
    typedef internal::impl::basic_stringbuf<layer1::string<32> > impl_type;
    typedef internal::streambuf<impl_type> streambuf_type;

    SECTION("basics")
    {
        layer1::string<32> str = "hi2u";

        internal::basic_istream<streambuf_type> _cin(str);

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
        layer1::string<32> str = "2 xyz";

        internal::basic_istream<streambuf_type> _cin(str);

        SECTION("integer conversion")
        {
            int val = - 1;

            _cin >> val;

            REQUIRE(_cin.good());
            REQUIRE(val == 2);

            _cin >> val;

            REQUIRE(_cin.good() == false);
            if(_cin) FAIL();
        }
        SECTION("istringstream")
        {
            experimental::istringstream<32> in = "123 456";

            int val;

            in >> val;

            REQUIRE(val == 123);

            in >> val;

            REQUIRE(val == 456);
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