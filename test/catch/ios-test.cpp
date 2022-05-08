//
// Created by malachi on 8/10/18.
//

#include <catch.hpp>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/string.h>
#include <estd/sstream.h>

// TODO: We need a better place to locate specialized overloads of << [this one
// is for the dynamic_array handler for istream]
#include <estd/internal/istream.h>
#include <estd/internal/ostream.h>

#include "test-data.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#ifndef __has_warning
#define __has_warning(x)    (true) // FAKE has warning
#endif

#if __has_warning("-Wunused-but-set-variable")
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif


using namespace estd;

//#define TEST_COUT

struct dummy_streambuf_impl : internal::impl::streambuf_base<std::char_traits<char> >
{
    typedef char char_type;
    typedef int int_type;
    typedef std::char_traits<char_type> traits_type;

    int xsputn(const char_type*, int len) { return len; }
    int xsgetn(char_type*, int len) { return len; }
};

typedef internal::streambuf<dummy_streambuf_impl> dummy_streambuf;

TEST_CASE("ios")
{
    const char raw_str[] = "raw 'traditional' output\n";
    CONSTEXPR int raw_str_len = sizeof(raw_str) - 1;

    /* This old cute and clever function-detector method no longer employed.
     * Ended up being more complicated than not using it in the end
    SECTION("SFINAE tests")
    {
        //typedef estd::internal::impl::native_streambuf<char, estd::internal::posix_stream_t, std::char_traits<char> >
        //        streambuf_impl_type;
        typedef estd::internal::impl::posix_streambuf streambuf_impl_type;
        typedef estd::internal::streambuf<streambuf_impl_type> streambuf_type;
        bool has_method = streambuf_type::has_sputc_method<streambuf_impl_type>::value;

        REQUIRE(has_method);
    } */
    SECTION("experimental tests")
    {
        internal::basic_ostream<dummy_streambuf> _cout;

        auto wrapped_out = experimental::convert(_cout);
        ostream& out = wrapped_out;
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
        SECTION("istream")
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

            _cin.sync();
        }
        SECTION("basic_streambuf_wrapped")
        {
            streambuf_type sb;
            internal::basic_streambuf_wrapped<streambuf_type&> sbw(sb);
            estd::basic_ostream<char> _cout(&sbw);

            _cout << "hi2u";

            const char* helper = sb.str().data();

            REQUIRE(sb.str() == "hi2u");
        }
        SECTION("wrapped_ostream")
        {
            streambuf_type sb;
            internal::basic_ostream<streambuf_type&> native_cout(sb);
            experimental::wrapped_ostream<streambuf_type&> _cout(sb);
            ostream& __cout = _cout;

            __cout << "hi2u";

            const char* helper = sb.str().data();

            REQUIRE(sb.str() == "hi2u");

            auto wrapped_out = experimental::convert(native_cout);
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
        estd::posix_ostream<char> _cout(*stdout);

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
    SECTION("layer1")
    {
        SECTION("stringbuf")
        {
            layer1::stringbuf<32> sb;
        }
        // NOTE: current layer1 ostringstream and istringstream is interesting
        // but I don't think representative of real use cases - i.e.
        // fusing the data and the format metadata together.  Smells like a solution
        // looking for a problem.  So, marking it as experimental
        SECTION("ostringstream")
        {
            experimental::ostringstream<32> out;

            int sz = sizeof(out);

            out << "hi2u";

            REQUIRE(out.rdbuf()->str() == "hi2u");

            out << endl;
        }
        SECTION("istringstream")
        {
            experimental::istringstream<32> in = "hi2u";

            REQUIRE(in.rdbuf()->str() == "hi2u");
        }
        SECTION("numeric test")
        {
            experimental::ostringstream<32> out;

            SECTION("base 10")
            {
                out << dec; // NOTE: This is default, but we want to be thorough

                int value = 2;

                out << "hi" << value++ << 'u';

                auto& s = out.rdbuf()->str();

                // NOTE: Works here but in ASF/Atmel land the 'value' gets
                // treated as a character
                REQUIRE(s == "hi2u");
            }
            SECTION("base 16")
            {
                out << hex;

                out << "hi" << 15 << 'u';

                auto& s = out.rdbuf()->str();

                // NOTE: Works here but in ASF/Atmel land the 'value' gets
                // treated as a character
                REQUIRE(s == "hifu");
            }
        }
        SECTION("tellp")
        {
            experimental::ostringstream<32> out;
            int value = 2;

            out << "hi" << value++ << 'u';

            int tellp = out.tellp();

            REQUIRE(tellp == 4);
        }
    }
    SECTION("layer2")
    {
        SECTION("stringbuf")
        {
            //layer2::stringbuf(raw_str);
        }
    }
    SECTION("spitting out various strings")
    {
        experimental::ostringstream<256> out;

        SECTION("layer2")
        {
            estd::layer2::const_string s = "hi2u";

            out << s;

            REQUIRE(out.rdbuf()->str().size() == 4);
        }
        SECTION("layer3")
        {
            estd::layer3::const_string s = "hi2u";

            out << s;

            REQUIRE(out.rdbuf()->str().size() == 4);
        }
    }
    SECTION("convenience typedefs for span streaming")
    {
        char buf[128];
        estd::span<char> span(buf);

        SECTION("in")
        {
            strcpy(buf, "hello\r\n");
            estd::layer1::string<32> s;

            estd::experimental::ispanstream in(span);

            in >> s;

            REQUIRE(s.starts_with("hello"));
        }
        SECTION("out")
        {
            estd::experimental::ospanstream out(span);

            out << "hi2u";

            REQUIRE(out.tellp() == 4);
        }
    }
}

#pragma GCC diagnostic pop
