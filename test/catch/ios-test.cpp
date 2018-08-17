//
// Created by malachi on 8/10/18.
//

#include <catch.hpp>

#include <estd/istream.h>
#include <estd/ostream.h>
#include <estd/string.h>

using namespace estd;

namespace estd {


// NOTE: This will work but doesn't filter specifically by string, which perhaps we want
template <class TStreambuf, class TBase, class TStringImpl>
internal::basic_istream<TStreambuf, TBase>& operator >>(
        internal::basic_istream<TStreambuf, TBase>& in,
        internal::dynamic_array<TStringImpl>& value)
{
    typedef typename internal::basic_istream<TStreambuf, TBase> istream_type;
    typedef typename estd::remove_reference<TStreambuf>::type impl_type;
    typedef typename impl_type::traits_type traits_type;
    typedef typename impl_type::char_type char_type;
    typedef typename impl_type::int_type int_type;

    in >> ws;

    //char_type* dest = value.lock();

    experimental::locale loc = in.getloc();

    value.clear();

    for(;;)
    {
        int_type ch = in.peek();

        if(ch == traits_type::eof())
        {
            in.setstate(istream_type::failbit | istream_type::eofbit);
            break;
        }
        else if(isspace((char_type)ch, loc)) break;

        //*dest++ = ch;

        // NOTE: += is defined and should have worked
        value.push_back((char_type)ch);
        //value += (char_type)ch;

        in.get();
    }

    //value.unlock();
    return in;
}


// NOTE: Experimental, only use if above doesn't satisfy things
/*
template <class TImpl, class TStringAllocator, class TStringPolicy>
internal::basic_istream<TImpl>& operator >>(internal::basic_istream<TImpl>& in,
                                            basic_string<
                                                typename TImpl::char_type,
                                                typename TImpl::traits_type,
                                                TStringAllocator,
                                                TStringPolicy>&
                                            value)
{
    return in;
}
*/

/*
 * Up against this error:

Undefined symbols for architecture x86_64:

"estd::internal::basic_istream<
    estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<
            estd::layer1::basic_string<
                char, 32ul, true, std::__1::char_traits<char>,
                estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
            >
        >
    >&,
    estd::internal::basic_ios<estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<
            estd::layer1::basic_string<
                char, 32ul, true, std::__1::char_traits<char>,
                estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
            >
        >
    >&>
>& estd::operator>><
    estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<
            estd::layer1::basic_string<
                char, 32ul, true, std::__1::char_traits<char>,
                estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
            >
        >
    >&,
    estd::layer1::basic_string<
        char, 32ul, true, std::__1::char_traits<char>,
        estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
    >
>(
    estd::internal::basic_istream<estd::internal::streambuf<
        estd::internal::impl::basic_stringbuf<
            estd::layer1::basic_string<
                char, 32ul, true, std::__1::char_traits<char>,
                estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
            >
        >
    >&,
    estd::internal::basic_ios<
        estd::internal::streambuf<
            estd::internal::impl::basic_stringbuf<
                estd::layer1::basic_string<
                    char, 32ul, true, std::__1::char_traits<char>,
                    estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false>
                >
            >
        >&>
    >&,
    estd::layer1::basic_string<
        char, 32ul, true, std::__1::char_traits<char>,
        estd::experimental::null_terminated_string_policy<std::__1::char_traits<char>, short, false> >&
)", referenced from:
      ____C_A_T_C_H____T_E_S_T____0() in ios-test.cpp.o

 */
/*
// NOTE: This isn't active yet.  Specialization still broken for prototype in istream.h
// pretty sure it has to do with us not getting the specialization signature *exactly right*
// so that when the compiler chooses the definition based on the prototype it matches up
// with the implementation here
template <class TImpl, class TBase, unsigned N, class CharT, class Traits, class Policy>
internal::basic_istream<TImpl, TBase>& operator >>(
        internal::basic_istream<TImpl, TBase>& in,
        layer1::basic_string<
            //typename TImpl::char_type,
            CharT,
            N, true,
            //typename TImpl::traits_type
            Traits,
            Policy
            >& s)
{
    return in;
}
*/
}

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

            // FIX: SHOULD be eof but we have our lingering null-termination coming back out
            // (which we shouldn't)
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
