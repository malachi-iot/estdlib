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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"


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

TEST_CASE("iostreams")
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
        SECTION("istream readsome")
        {
            char localbuf[128];
            layer1::string<32> str = "hi2u";

            internal::basic_istream<streambuf_type> _cin(str);

            estd::streamsize read_back = _cin.readsome(localbuf, str.length());

            REQUIRE(read_back == str.length());
            REQUIRE(localbuf[0] == str[0]);
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
    SECTION("impl::out_span_streambuf")
    {
        // FIX: Not quite there because span constructor won't switch between Extent and
        // non Extent version for explicit buffer just yet
        constexpr int val_size = 32;
        uint32_t val[val_size];
        uint8_t test1[] = { 0, 1, 2, 3 };

        val[0] = 0;

        typedef estd::internal::impl::out_span_streambuf<uint8_t> streambuf_impl_type;

        SECTION("initialization")
        {
            typedef estd::internal::streambuf<streambuf_impl_type> streambuf_type;

            SECTION("array init")
            {
                uint8_t buffer[32];

                streambuf_type streambuf(buffer);

                streambuf.sputn((uint8_t*)"abc", 3);

                REQUIRE(streambuf.pos() == 3);
                REQUIRE(buffer[0] == 'a');
            }
        }
        SECTION("full (non impl) version")
        {
            // 'test' actually sits dormant and does nothing.  artifact of previous test
            // approach
            estd::internal::impl::out_span_streambuf<uint8_t> test((uint8_t*)&val[0], 32);

            // NOTE: Wanted to do a ref version here but not sure if I actually
            // want that to be a supported technique
            // NOTE: This and many other of the '32' used around here are wrong, since
            // that's the uint32_t-oriented size
            estd::internal::streambuf<decltype(test)> sb((uint8_t*)&val[0], 32);

            sb.sputc(0);
            sb.sputc(1);
            sb.sputc(2);
            sb.sputc(3);

            // TODO: Make this endian-inspecific.  For the time being though I anticipate
            // these unit tests only running on x64 machines
            REQUIRE(val[0] == 0x03020100);

            int sz = sizeof(sb);

            // TODO: Re-enable this, upgraded out_span_streambuf and broke old sizing
            //REQUIRE(sz == sizeof(estd::span<uint32_t, 32>) + sizeof(size_t));

            //REQUIRE(sb.epptr() - sb.pbase() == 32 * 4);
        }
        SECTION("non-constexpr size version")
        {
            estd::internal::streambuf<streambuf_impl_type> sb((uint8_t*)&val[0], 32);

            sb.sputc(0);
            sb.sputc(1);
            sb.sputc(2);
            sb.sputc(3);

            // DEBT: Make this non-endian-specific
            REQUIRE(val[0] == 0x03020100);

            int sz = sizeof(sb);

            // TODO: Re-enable this, upgraded out_span_streambuf and broke old sizing
            // TODO: see why it grew from 24 to 32 bytes also (maybe because we inherit pos now
            // and before it was packing it? doubtful.  Probably because pos is this more complicated
            // pos_type from native char_traits)
            //REQUIRE(sz == sizeof(estd::span<uint32_t>) + sizeof(size_t));

            sb.pubseekoff(-1, estd::ios_base::cur);

            sb.sputc(4);

            // DEBT: Make this non-endian-specific
            REQUIRE(val[0] == 0x04020100);
        }
        SECTION("ostream usage (complex initialization)")
        {
            typedef estd::internal::impl::out_span_streambuf<uint32_t> sb_impl_type;
            typedef estd::internal::streambuf<sb_impl_type> sb_type;

            // Successfully cascades down 'val, 32' all the way down to out_span_streambuf
            estd::internal::basic_ostream<sb_type> out(val, 32);

            REQUIRE(out.rdbuf()->value().size() == 32);
        }
        SECTION("experimental")
        {
            SECTION("setbuf")
            {
                streambuf_impl_type sb((uint8_t*)&val[0], val_size);

                uint8_t* data = sb.pptr();

                // 32-bit 0 will not match 8-bit 0, 1, 2, 3 on 2nd byte
                // regardless of endianess (unless we have some flavor of
                // the double-swap little endian looking like 1, 0, 3, 2)
                REQUIRE(data[1] != test1[1]);

                sb.setbuf_experimental(test1, sizeof test1);

                data = sb.pptr();

                REQUIRE(data[0] == test1[0]);
                REQUIRE(data[1] == test1[1]);
                REQUIRE(data[2] == test1[2]);
                REQUIRE(data[3] == test1[3]);
            }
        }
    }
    SECTION("in_span_streambuf")
    {
        char buf[128];

        typedef estd::internal::impl::in_span_streambuf<const char> sb_impl_type;
        typedef estd::internal::streambuf<sb_impl_type> sb_type;

        sb_type sb(raw_str);

        SECTION("sgetn")
        {
            estd::streamsize read_back = sb.sgetn(buf, sizeof(buf));

            REQUIRE(sb.eback() == raw_str);
        }
        SECTION("sbumpc")
        {
            REQUIRE(sb.sbumpc() == raw_str[0]);
            REQUIRE(sb.sbumpc() == raw_str[1]);
            REQUIRE(sb.sbumpc() == raw_str[2]);
        }
        SECTION("pubseekoff")
        {
            int new_pos;

            new_pos = sb.pubseekoff(5, ios_base::cur);
            REQUIRE(new_pos == 5);
            new_pos = sb.pubseekoff(5, ios_base::cur);
            REQUIRE(new_pos == 10);
            REQUIRE(sb.sgetc() == raw_str[new_pos]);
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
    SECTION("pubsync - method finding")
    {
        struct pubsync_only_streambuf_impl :
                internal::impl::streambuf_base<estd::char_traits<char> >
        {
            int sync() const { return 7; }
        };

        layer1::stringbuf<32> sb1;

        internal::streambuf<pubsync_only_streambuf_impl> sb2;

        REQUIRE(sb1.pubsync() == 0);
        REQUIRE(sb2.pubsync() == 7);
    }
}

#pragma GCC diagnostic pop
