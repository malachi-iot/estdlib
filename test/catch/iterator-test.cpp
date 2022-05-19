#include <catch.hpp>

#include <estd/iterator.h>
#include <estd/sstream.h>

TEST_CASE("iterator")
{
    SECTION("istreambuf_iterator")
    {
        // FIX: layer3::stringbuf is fiddly.  Needs:
        // - better const awareness
        // -- maybe a specialization replacing layer3::const_string into layer3::basic_string<const char>
        //estd::layer3::stringbuf in("hello");
        estd::layer3::stringbuf::size_type sz = 11;
        estd::layer3::stringbuf in((char*)"hello world", sz, sz);

        SECTION("end-of-stream")
        {
            estd::istreambuf_iterator<estd::layer2::stringbuf> it, end;

            REQUIRE(it == end);
        }
        SECTION("misc stringbuf")
        {
            estd::istreambuf_iterator<estd::layer3::stringbuf> it(&in);

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
                    estd::istreambuf_iterator<estd::layer3::stringbuf> it(&in), end;

                    for (i = sz; i > 0; --i, ++it);

                    REQUIRE(it == end);
                }
                SECTION("postfix")
                {
                    estd::istreambuf_iterator<estd::layer3::stringbuf> it(&in), end;

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
        estd::experimental::istringstream<32> istream;
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
}