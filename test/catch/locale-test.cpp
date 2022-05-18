#include <catch.hpp>

#include <estd/algorithm.h>
#include <estd/iterator.h>
#include <estd/locale.h>
#include <estd/sstream.h>

#include "test-data.h"

#include <estd/internal/locale/cbase.h>

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
        typename internal::is_compatible_encoding<
            estd::internal::encodings::ASCII, encoding
            >::type>
{

};


// Because catch doesn't like our pseudo constexpr static members
static const auto goodbit = ios_base::goodbit;
static const auto failbit = ios_base::failbit;
static const auto eofbit = ios_base::eofbit;


template <class TInt, class TIStream>
void num_get_simple_test(const char* in, TInt expected, TIStream& fmt)
{
    TInt v;
    ios_base::iostate state = goodbit;

    num_get<char, const char*> n;

    n.get(in, in + strlen(in), fmt, state, v);

    REQUIRE(state == eofbit);
    REQUIRE(v == expected);
}

// As per
// https://www.translate.com/klingon-english
struct klingon_locale : locale::classic_type
{
    typedef estd::locale::classic_type base_type;
};

struct copycat_classic_locale :
    estd::internal::locale<locale::iso::en_US, locale::encodings::ASCII> {};

namespace estd {

template <>
struct ctype<char, klingon_locale> :
    numpunct<char, klingon_locale::base_type>
{

};

template <>
struct numpunct<char, klingon_locale> :
    numpunct<char, klingon_locale::base_type>
{
    static layer2::const_string truename() { return "teH"; }
    static layer2::const_string falsename() { return "ngeb"; }
};

}



TEST_CASE("locale")
{
    locale::type<locale::iso::en_US,
        locale::encodings::UTF8> l;
    internal::locale<internal::locale_code::fr_FR,
        internal::encodings::UTF8> l_fr;
    internal::locale<internal::locale_code::en_US,
            internal::encodings::ASCII> l_ASCII;

    // Not truly necessary (classic_type is fully static) but we do have the API for those
    // who want it
    locale::classic_type l_classic = locale::classic();

    SECTION("isspace")
    {
        REQUIRE(isspace(' ', l));
    }
    SECTION("num_get")
    {
        ios_base::iostate state = ios_base::goodbit;
        //ios_base fmt;
        // DEBT: Need to do this because we are basing on basic_ios, not ios_base, due
        // to moved getloc().  Can be any estd istream here
        estd::internal::basic_istream<layer1::stringbuf<32>> fmt;
        ios_base fmt2;
        long v = -1;

        SECTION("simple source")
        {
            num_get<char, const char*> n;

            SECTION("dec")
            {
                SECTION("basic")
                {
                    num_get_simple_test("123", 123, fmt);
                }
                SECTION("signed")
                {
                    num_get_simple_test("-123", -123, fmt);
                }
                SECTION("unsigned")
                {
                    num_get_simple_test("123", (unsigned)123, fmt);
                }
            }
            SECTION("hex")
            {
                fmt.setf(ios_base::hex, ios_base::basefield);

                num_get_simple_test("FF", 255, fmt);
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
                    //REQUIRE(out == in + 2);
                }
            }
        }
        SECTION("complex iterator")
        {
            SECTION("istreambuf_iterator")
            {
                typedef estd::layer1::stringbuf<32> streambuf_type;
                streambuf_type sb = test::str_uint1;
                typedef estd::istreambuf_iterator<streambuf_type> iterator_type;
                iterator_type it(sb), end;
                num_get<char, iterator_type> n;

                auto result = n.get(it, end, fmt, state, v);

                REQUIRE(state == eofbit);
                REQUIRE(v == test::uint1);
                REQUIRE(result == end);
            }
            SECTION("istreambuf_iterator (deduced)")
            {
                // mimics the example at
                // https://en.cppreference.com/w/cpp/locale/num_get/get
                typedef estd::layer1::stringbuf<32> streambuf_type;
                streambuf_type sb = test::str_uint1;
                // Since istreambuf_iterators in our world require knowledge of the streambuf_type
                // we deviate spec of num_get in this instance to accept a streambuf_type instead of
                // just char_type, iter_type
                auto n = use_facet<num_get<streambuf_type>>(fmt.getloc());

                n.get(sb, {}, fmt, state, v);

                REQUIRE(state == eofbit);
                REQUIRE(v == test::uint1);
            }
        }
        SECTION("iterated")
        {
            bool done;

            SECTION("core")
            {
                const char* in = "123";

                iterated::num_get<10, char, locale::classic_type> n;

                done = n.get(*in++, state, v);
                REQUIRE(!done);
                done = n.get(*in++, state, v);
                REQUIRE(!done);
                done = n.get(*in++, state, v);
                REQUIRE(!done);
                REQUIRE(v == 123);
                done = n.get(*in++, state, v);
                REQUIRE(done);
                REQUIRE(state == goodbit);
                REQUIRE(v == 123);
            }
            SECTION("helper method")
            {
                const char* in = "-123";
                iterated::num_get<10, char, locale::classic_type> n;
                const char* end = in + strlen(in);

                do
                {
                    done = n.get(in, end, state, v);
                }
                while(!done);

                REQUIRE(state == eofbit);
                REQUIRE(done);
                REQUIRE(v == -123);
            }
            SECTION("bool")
            {
                iterated::bool_get<char, locale::classic_type, true> n;
                const char* in = "true";
                const char* end = in + 4;
                bool v = false;
                bool done;

                do
                {
                    done = n.get(in, end, state, v);
                }
                while(!done);

                REQUIRE(v == true);
            }
        }
        SECTION("non standard")
        {
            const char* in = "-123";

            auto n = use_facet<num_get<char, const char*> >(l);

            // This call looks identical to standard call, but it deduces locale from
            // use_facet rather than fmt.  I bet if those two are different in std
            // version of things, it's undefined behavior.  So likely a very safe move here
            n.get(in, in + strlen(in), fmt2, state, v);
        }
    }
    SECTION("cbase")
    {
        SECTION("base 8")
        {
            cbase<char, 8, locale::classic_type> facet;

            REQUIRE(*facet.from_char('7') == 7);
            REQUIRE(!facet.from_char('9').has_value());
        }
        SECTION("base 10")
        {
            cbase<char, 10, locale::classic_type> facet;

            REQUIRE(*facet.from_char('2') == 2);
            REQUIRE(facet.from_char('A').has_value() == false);

            REQUIRE(facet.is_in_base('9') == true);
            REQUIRE(facet.is_in_base('F') == false);
        }
        SECTION("base 16")
        {
            cbase<char, 16, locale::classic_type> facet;

            REQUIRE(*facet.from_char('A') == 10);
            REQUIRE(facet.from_char('.').has_value() == false);

            REQUIRE(facet.is_in_base('F') == true);
            REQUIRE(facet.is_in_base('G') == false);
        }
    }
    SECTION("facet")
    {
        SECTION("use_facet")
        {
            SECTION("ctype")
            {
                SECTION("char")
                {
                    auto f = use_facet<ctype<char>>(l);

                    REQUIRE(f.is(ctype_base::upper, 'a') == false);
                    REQUIRE(f.is(ctype_base::upper | ctype_base::lower | ctype_base::xdigit, 'A') == true);
                }
            }
            SECTION("num_get")
            {
                SECTION("unusual input")
                {
                    // mirrors what happens in our unity test
                    const char* input = "123 456/789";

                    unsigned val;
                    ios_base::iostate err = ios_base::goodbit;

                    typedef experimental::istringstream<64> istream_type;
                    //estd::internal::basic_istream<layer1::basic_stringbuf<char, 32>> istream(input);
                    istream_type istream(input);
                    typedef estd::istreambuf_iterator<decltype(istream)::streambuf_type>
                        iterator_type;
                    auto facet = use_facet<num_get<char, iterator_type> > (istream.getloc());

                    iterator_type it(istream), end;

                    it = facet.get(it, end, istream, err, val);

                    REQUIRE(val == 123);
                    REQUIRE(err == goodbit);

                    // FIX: istreambuf_iterator is not yet up to this task, it's always in proxy mode
                    // which means it doesn't pick up non-iterator movement through the streambuf
                    //istream >> ws;
                    // instead, we manually skip the space
                    ++it;

                    it = facet.get(it, end, istream, err, val);

                    // FIX: Fail, we get 0
                    REQUIRE(err == goodbit);
                    REQUIRE(val == 456);
                }
                SECTION("wchar")
                {
                    auto f = use_facet<num_get<wchar_t, const wchar_t*>>(l);

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
            }
            SECTION("numpunct")
            {
                SECTION("en")
                {
                    auto f = use_facet<numpunct<char>>(l);

                    REQUIRE(f.truename() == "true");
                }
                SECTION("fr")
                {
                   REQUIRE(use_facet<numpunct<char>>(l_fr).truename() == "vrai");
                }
                SECTION("en")
                {
                    REQUIRE(use_facet<numpunct<char>>(l_ASCII).truename() == "true");
                }
                SECTION("classic")
                {
                    REQUIRE(use_facet<numpunct<char>>(l_classic).truename() == "true");
                }
            }
            SECTION("moneypunct")
            {
                auto f = use_facet<moneypunct<char>>(l);

                REQUIRE(f.curr_symbol() == "$");

                auto f2 = use_facet<moneypunct<char, true>>(l);

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
            SECTION("helpers")
            {
                REQUIRE(internal::is_compatible_with_classic_locale<decltype(l)>::value);
                REQUIRE(internal::is_compatible_with_classic_locale<decltype(l_ASCII)>::value);
                REQUIRE(internal::is_compatible_with_classic_locale<decltype(l_classic)>::value);

                // FIX: This should be true, but comes back false
                //REQUIRE(internal::is_compatible_with_classic_locale<copycat_classic_locale>::value);

                // Actually, at the moment klingon_locale IS compatible with classic, though
                // this test actually thinks it isn't
                //REQUIRE(!internal::is_compatible_with_classic_locale<klingon_locale>::value);
            }
        }
    }
    SECTION("custom locale")
    {
        klingon_locale l;

        auto facet = use_facet<numpunct<char> >(l);
        //const char* val = facet.truename().data();

        REQUIRE(facet.truename() == "teH");
    }
}