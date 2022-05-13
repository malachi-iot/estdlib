#include <catch.hpp>

#include <estd/algorithm.h>
#include <estd/iterator.h>
#include <estd/locale.h>
#include <estd/sstream.h>

#include "test-data.h"

using namespace estd;

template <internal::encodings::values encoding, bool enabled>
struct test_fallthrough;

template <internal::encodings::values encoding>
struct test_fallthrough2;

template <internal::encodings::values encoding>
struct test_fallthrough<encoding, false> {};

template <internal::encodings::values encoding, typename enabled = void>
struct test_fallthrough3;

/*
template <internal::encodings::values encoding>
struct test_fallthrough3<encoding, void> {}; */



// "template argument ... involves template parameter" (it doesn't like pushing a value through)
/*
template <internal::encodings::values encoding>
struct test_fallthrough<
        experimental::is_compatible_encoding<internal::encodings::ASCII, encoding>::value
        >
{

}; */

/*
template <internal::encodings::values encoding>
struct test_fallthrough2<experimental::is_compatible_encoding2<encoding>::value> :

{

}; */

/*
 * This works, but I think I prefer our custom specialization below
 */

/*
template <internal::encodings::values encoding>
struct test_fallthrough3<encoding, estd::enable_if_t<encoding == estd::internal::encodings::ASCII> >
{

}; */

template <internal::encodings::values encoding>
struct test_fallthrough3<encoding,
        typename experimental::_internal::is_compatible_encoding<
            estd::internal::encodings::ASCII, encoding
            >::type>
{

};


TEST_CASE("locale")
{
    experimental::locale<experimental::locale_code::en_US,
        internal::encodings::UTF8> l;
    experimental::locale<experimental::locale_code::fr_FR,
        internal::encodings::UTF8> l_fr;
    experimental::locale<experimental::locale_code::en_US,
            internal::encodings::ASCII> l_ASCII;
    auto l_classic = experimental::locale_base::classic();
    auto goodbit = ios_base::goodbit;
    auto failbit = ios_base::failbit;
    auto eofbit = ios_base::eofbit;

    SECTION("isspace")
    {
        REQUIRE(experimental::isspace(' ', l));
    }
    SECTION("num_get")
    {
        ios_base::iostate state = ios_base::goodbit;
        //ios_base fmt;
        // DEBT: Need to do this because we are basing on basic_ios, not ios_base, due
        // to moved getloc().  Can be any estd istream here
        estd::internal::basic_istream<layer1::stringbuf<32>> fmt;
        long v = -1;

        SECTION("simple source")
        {
            experimental::num_get<char, const char*> n;

            SECTION("dec")
            {
                SECTION("basic")
                {
                    const char* in = "123";

                    n.get(in, in + 3, fmt, state, v);

                    REQUIRE(state == eofbit);
                    REQUIRE(v == 123);
                }
                SECTION("signed")
                {
                    const char* in = "-123";

                    n.get(in, in + 4, fmt, state, v);

                    REQUIRE(state == eofbit);
                    REQUIRE(v == -123);
                }
                SECTION("unsigned")
                {
                    const char* in = "123";
                    unsigned _v;

                    n.get(in, in + 3, fmt, state, _v);

                    REQUIRE(state == eofbit);
                    REQUIRE(_v == 123);
                }
            }
            SECTION("hex")
            {
                const char* in = "FF";

                fmt.setf(ios_base::hex, ios_base::basefield);
                n.get(in, in + 2, fmt, state, v);

                REQUIRE(state == eofbit);
                REQUIRE(v == 255);
            }
            SECTION("erroneous")
            {
                const char* in = "whoops";

                n.get(in, in + 6, fmt, state, v);

                REQUIRE(state == failbit);
                // As per https://en.cppreference.com/w/cpp/locale/num_get/get "Stage 3: conversion and storage"
                // "If the conversion function fails [...] 0 is stored in v" and also
                // "C++98/C++03 left [v] unchanged [...]. Such behavior is corrected by [...] C++11"
                REQUIRE(v == 0);
            }
            SECTION("unusual delimiter")
            {
                // Although I couldn't find it documented anywhere, hands on testing indicates
                // f.get() treats any unrecognized character as a numeric delimiter, and that's
                // how we end up with 'goodbit'* results --- * being that C++11 actually returns
                // whatever you pass in, which for us is goodbit
                const char* in = "123/";

                const char* output_it = n.get(in, in + 6, fmt, state, v);
                REQUIRE(state == goodbit);
                REQUIRE(v == 123);
                REQUIRE(*output_it == '/');
            }
            SECTION("bool numeric")
            {
                const char* in = "1";
                bool _v;

                n.get(in, in + 1, fmt, state, _v);
                REQUIRE(state == eofbit);
                REQUIRE(_v == true);
            }
            SECTION("bool alpha")
            {
                fmt.setf(ios_base::boolalpha);
                bool _v;

                SECTION("true")
                {
                    const char* in = "true ";

                    n.get(in, in + 5, fmt, state, _v);
                    REQUIRE(state == goodbit);
                    REQUIRE(_v == true);
                }
                SECTION("false")
                {
                    const char* in = "false";

                    n.get(in, in + 5, fmt, state, _v);
                    REQUIRE(state == eofbit);
                    REQUIRE(_v == false);
                }
                SECTION("error")
                {
                    const char* in = "faKse";
                    const char* out = n.get(in, in + 5, fmt, state, _v);
                    REQUIRE(state == failbit);
                    REQUIRE(_v == false);

                    // DEBT: Not quite sure if this is undefined, or even correct
                    REQUIRE(out == in + 2);
                }
            }
        }
        SECTION("complex iterator")
        {
            SECTION("istreambuf_iterator")
            {
                typedef estd::layer1::stringbuf<32> streambuf_type;
                streambuf_type sb = test::str_uint1;
                typedef estd::experimental::istreambuf_iterator<streambuf_type> iterator_type;
                iterator_type it(sb), end;
                experimental::num_get<char, iterator_type> n;

                auto result = n.get(it, end, fmt, state, v);

                REQUIRE(state == eofbit);
                REQUIRE(v == test::uint1);
                REQUIRE(result == end);
            }
        }
    }
    SECTION("use_facet")
    {
#if UNUSED
        constexpr char c = 'a';
        SECTION("ctype 1")
        {
            typedef experimental::ctype<char,
                experimental::locale<experimental::locale_code::en_US, internal::encodings::UTF8>>
                ctype_type;
            char result = experimental::use_facet<ctype_type>(l).widen(c);
            REQUIRE(result == c);
        }
        SECTION("ctype 2")
        {
            auto _c = experimental::use_facet_ctype<char>(l);

            REQUIRE(_c.widen(c) == c);
            REQUIRE(!_c.is(estd::experimental::ctype_base::digit, c));
        }
        SECTION("ctype3 ")
        {
            auto f =
                experimental::use_facet3<experimental::ctype_test<char>>(l);

            REQUIRE(!f.is(estd::experimental::ctype_base::digit, c));
            // Not yet implemented
            //REQUIRE(f.is(estd::experimental::ctype_base::alpha, c));
        }
#endif
        SECTION("use_facet4")
        {
            using namespace estd::experimental;

            SECTION("ctype")
            {
                SECTION("char")
                {
                    auto f = use_facet4<ctype<char>>(l);

                    REQUIRE(f.is(ctype_base::upper, 'a') == false);
                    REQUIRE(f.is(ctype_base::upper | ctype_base::lower | ctype_base::xdigit, 'A') == true);
                }
            }
            SECTION("num_get")
            {
                auto f = use_facet4<num_get<wchar_t, const wchar_t*>>(l);

                const wchar_t* number = L"1234";
                int value;
                ios_base::iostate state = ios_base::goodbit;

                // DEBT: Only used for format and locale specification
                estd::internal::basic_istream<layer1::basic_stringbuf<wchar_t, 32>> fmt;

                f.get(number, number + 4, fmt, state, value);

                REQUIRE(state == eofbit);
                // TODO: Works, but only because unicode comfortably narrows back down to regular
                // char during char_base_traits usage
                REQUIRE(value == 1234);
            }
            SECTION("numpunct")
            {
                SECTION("en")
                {
                    auto f = use_facet4<numpunct<char>>(l);

                    REQUIRE(f.truename() == "true");
                }
                SECTION("fr")
                {
                   REQUIRE(use_facet4<numpunct<char>>(l_fr).truename() == "vrai");
                }
                SECTION("en")
                {
                    REQUIRE(use_facet4<numpunct<char>>(l_ASCII).truename() == "true");
                }
                SECTION("classic")
                {
                    REQUIRE(use_facet4<numpunct<char>>(l_classic).truename() == "true");
                }
            }
            SECTION("moneypunct")
            {
                auto f = use_facet4<moneypunct<char>>(l);

                REQUIRE(f.curr_symbol() == "$");

                auto f2 = use_facet4<moneypunct<char, true>>(l);

                REQUIRE(f2.curr_symbol() == "USD ");
            }
        }
        SECTION("compatible locale")
        {
            using namespace estd::experimental;

            //constexpr internal::encodings::values v = is_compatible_encoding<internal::encodings::ASCII, internal::encodings::UTF8>::value;

            //REQUIRE(v == internal::encodings::UTF8);

            SECTION("fallthrough")
            {
                test_fallthrough3<internal::encodings::ASCII> ft1;
                test_fallthrough3<internal::encodings::UTF8> ft2;
                // Won't compile, and that's correct
                //test_fallthrough3<internal::encodings::UTF16> ft3;
            }
        }
    }
}