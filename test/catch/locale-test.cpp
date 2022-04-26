#include <catch.hpp>

#include <estd/algorithm.h>
#include <estd/iterator.h>
#include <estd/locale.h>
#include <estd/sstream.h>

#include "test-data.h"

using namespace estd;

TEST_CASE("locale")
{
    experimental::locale<experimental::locale_code::en_US,
        internal::encodings::UTF8> l;

    SECTION("isspace")
    {
        REQUIRE(experimental::isspace(' ', l));
    }
    SECTION("num_get")
    {
        auto goodbit = ios_base::goodbit;
        auto failbit = ios_base::failbit;
        auto eofbit = ios_base::eofbit;
        ios_base::iostate state = ios_base::goodbit;
        ios_base fmt;
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
        constexpr char c = 'a';
        SECTION("ctype 1")
        {
            typedef experimental::ctype<experimental::locale_code::en_US, internal::encodings::UTF8, char>
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
    }
}