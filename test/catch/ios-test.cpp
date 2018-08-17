//
// Created by malachi on 8/10/18.
//

#include <catch.hpp>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/string.h>

// TODO: We need a better place to locate specialized overloads of << [this one
// is for the dynamic_array handler for istream]
#include <estd/internal/istream.h>
#include <estd/internal/ostream.h>

using namespace estd;

TEST_CASE("iostreams")
{
    char raw_str[] = "raw 'traditional' output\n";
    CONSTEXPR int raw_str_len = sizeof(raw_str) - 1;

    SECTION("SFINAE tests")
    {
        typedef estd::internal::impl::native_streambuf<char, estd::TEST_STREAM_T, std::char_traits<char> >
                streambuf_impl_type;
        typedef estd::internal::streambuf<streambuf_impl_type> streambuf_type;
        bool has_method = streambuf_type::has_sputc_method<streambuf_impl_type>::value;

        REQUIRE(has_method);
    }
    SECTION("basic_streambuf test")
    {
        // NOTE: posix_streambuf probably technically should be able to take stdin AND stdout...
        //posix_streambuf<char> psb(*stdout);
        //internal::basic_streambuf_wrapped<posix_streambuf<char>& > sbw(psb);

        internal::basic_streambuf_wrapped<posix_streambuf<char> > sbw(*stdout);
        basic_streambuf<char>& sb = sbw;

#ifdef TEST_COUT
        sb.sputn(raw_str, sizeof(raw_str) - 1);
#endif
    }
    SECTION("internal basic_stringbuf test")
    {
        typedef internal::impl::basic_stringbuf<layer1::string<32> > impl_type;
        typedef internal::streambuf<impl_type> streambuf_type;

        SECTION("impl")
        {
            impl_type sb;

            sb.xsputn(raw_str, sizeof (raw_str) - 1);

            REQUIRE(sb.str() == raw_str);
        }
        SECTION("internal")
        {
            streambuf_type sb;

            sb.sputn(raw_str, sizeof (raw_str) - 1);

            REQUIRE(sb.str() == raw_str);
        }
        SECTION("ostream / istream")
        {
            char localbuf[128];
            layer1::string<32> str = "hi2u";

            // wrap with a (basically) real ostream
            internal::basic_ostream<streambuf_type> _cout;
            // extract the internal inline-value rdbuf
            streambuf_type* rdbuf = _cout.rdbuf();

            _cout << raw_str;

            REQUIRE(_cout.rdbuf()->str() == raw_str);

            //_cout.rdbuf()->str().clear();

            _cout << '!';
            _cout << str;

            // wrap reference to streambuf with (basically) real istream
            internal::basic_istream<streambuf_type&> _cin(*rdbuf);

            // pull data out, using same rdbuf as _cout
            _cin.read(localbuf, raw_str_len);

            //localbuf[10] = 0;

            //REQUIRE(localbuf[0] == raw_str[0]);
            REQUIRE(memcmp(localbuf, raw_str, raw_str_len) == 0);

            // FIX: Doesn't work, because internal pointers don't advance
            REQUIRE(_cin.get() == '!');
            //_cin >> localbuf;
        }
        SECTION("whitespace on input")
        {
            // wrap with a (basically) real ostream
            internal::basic_ostream<streambuf_type> _cout;
            // extract the internal inline-value rdbuf
            streambuf_type* rdbuf = _cout.rdbuf();
            // wrap reference to streambuf with (basically) real istream
            internal::basic_istream<streambuf_type&> _cin(*rdbuf);

            layer1::string<32> str;
            layer1::string<32> whitespace_str = "   ";

            // TODO: organize this, << operator is organized totally differently than >>
            // operator (<< is in string.h, >> is in internal/istream.h)
            _cout << whitespace_str;
            _cout << "     lots of whitespace!  ";

            //_cin >> estd::ws;
            _cin >> str;

            const char* helper = str.clock();

            REQUIRE(str == "lots");

            _cin >> str;

            helper = str.clock();

            REQUIRE(str == "of");

            _cin >> str;

            helper = str.clock();

            REQUIRE(str == "whitespace!");

            REQUIRE(_cin.good());

            _cin >> str;

            REQUIRE(_cin.eof());
        }
    }
    SECTION("cin")
    {
        // limited testing since an automated test shouldn't pause for input
        estd::istream _cin(*stdin);

        // POSIX in doesn't reveal in_avail
        //_cin.rdbuf()->in_avail();
    }
    SECTION("cout")
    {
        //estd::basic_streambuf<char> streambuf(*stdout);
        estd::ostream _cout(*stdout);

#ifdef TEST_COUT

        int value = 123;

        _cout << "Got here #";
        _cout << value;
        _cout << estd::endl;

        //_cout.put('!');
        _cout << '!';

        SECTION("estd::string")
        {
            layer1::string<20> s = "hi";

            _cout << s << endl;
        }
#endif
    }
}
