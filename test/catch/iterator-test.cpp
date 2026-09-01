#include <catch2/catch_all.hpp>

#include <estd/iterator.h>
#include <estd/sstream.h>

#pragma GCC diagnostic ignored "-Wunused-variable"

TEST_CASE("iterator")
{
    SECTION("istreambuf_iterator")
    {
        // FIX: layer3::stringbuf is fiddly.  Needs:
        // - better const awareness
        // -- maybe a specialization replacing layer3::const_string into layer3::basic_string<const char>
        //estd::layer3::stringbuf in("hello");
        using streambuf_legacy_type = estd::layer3::stringbuf;
        /*
         * Almost, but something about the init_t/InitParam chain gets confused
         * when const char is used here.  Noting that in https://github.com/malachi-iot/estdlib/issues/223
        using streambuf_good_type = // NOLINT
            estd::layer3::basic_stringbuf<
                const char, false,
                estd::detail::char_traits<const char>>;
        using streambuf_type = streambuf_good_type; */
        using streambuf_type = streambuf_legacy_type;
        estd::layer3::stringbuf::size_type sz = 11;
        const char* s = "hello world";
        // FIX: Relating to the FIX above, clearly don't be casting away const here
        streambuf_type in((char*)s, (char*)s + sz, sz);
        //streambuf_type in(s, s + sz, sz);

        SECTION("end-of-stream")
        {
            estd::istreambuf_iterator<estd::layer2::stringbuf> it, end;

            REQUIRE(it == end);
        }
        SECTION("misc stringbuf")
        {
            estd::istreambuf_iterator<streambuf_type> it(&in);

            SECTION("characters")
            {
                REQUIRE(*it++ == 'h');
                REQUIRE(*it++ == 'e');
                REQUIRE(*it++ == 'l');
                REQUIRE(*it == 'l');
                REQUIRE(*++it == 'o');
                REQUIRE(*++it == ' ');
                REQUIRE(*it++ == ' ');
                REQUIRE(*it++ == 'w');
            }
            SECTION("eol")
            {
                int i;

                SECTION("prefix")
                {
                    estd::istreambuf_iterator<streambuf_type> it(&in), end;

                    for (i = sz; i > 0; --i, ++it);

                    REQUIRE(it == end);
                }
                SECTION("postfix")
                {
                    estd::istreambuf_iterator<streambuf_type> it(&in), end;

                    for (i = sz; i > 0; i--, it++);

                    REQUIRE(it == end);
                    REQUIRE(it++ == end);
                }
            }
        }
        SECTION("ostream")
        {
            estd::layer1::stringbuf<128> sb;
            estd::experimental::ostreambuf_iterator<estd::layer1::stringbuf<128>> it(&sb);

            it++ = 'h';
            it++ = 'e';
            it++ = 'l';
            it = 'l';
            ++it = 'o'; // ostreambuf_iterator treats ++ as a no-op, so this will work
            ++it;

            REQUIRE(sb.str() == "hello");
        }
    }
    SECTION("istream_iterator")
    {
        estd::layer1::istringstream<32> istream;
        ESTD_CPP_ATTR_MAYBE_UNUSED
        estd::experimental::istream_iterator<unsigned, decltype(istream)> in(istream), end;
    }
    SECTION("filter_iterator")
    {
        auto it = estd::experimental::make_filter_iterator(
            [](char c)
            {
                return c == '/' ? 0 : c;
            },
            "hi2u/");

        //estd::layer1::string<32>(it, 0);
        estd::layer1::string<32> s;

        while(*it != 0)
        {
            s += *it++;
        }

        REQUIRE(s == "hi2u");


        //estd::experimental::filter_iterator
    }
    SECTION("ostream_iterator")
    {
        estd::layer1::ostringstream<128> out;
        estd::experimental::ostream_iterator<char, decltype(out)> it{out};

        *it++ = 'H';
        *it++ = 'i';

        REQUIRE(out.rdbuf()->str() == "Hi");
    }
}
