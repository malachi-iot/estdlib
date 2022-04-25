#include <catch.hpp>

#include <estd/algorithm.h>
#include <estd/iterator.h>
#include <estd/locale.h>
#include <estd/sstream.h>

#include "test-data.h"

using namespace estd;

TEST_CASE("locale")
{
    experimental::locale l;

    SECTION("isspace")
    {
        REQUIRE(experimental::isspace(' ', l));
    }
    SECTION("num_get")
    {
        ios_base::iostate state;
        ios_base holder;
        long v = 0;

        SECTION("simple source")
        {
            experimental::num_get<char, const char*> n;

            SECTION("decimal")
            {
                const char* in = "123";

                n.get(in, in + 3, state, holder, v);

                REQUIRE(v == 123);
            }
            SECTION("hex")
            {
                const char* in = "FF";

                holder.setf(ios_base::hex, ios_base::basefield);
                n.get(in, in + 2, state, holder, v);

                REQUIRE(v == 255);
            }
            SECTION("erroneous")
            {
                const char* in = "whoops";

                holder.setf(ios_base::hex, ios_base::basefield);
                n.get(in, in + 6, state, holder, v);

                //REQUIRE(holder.fail());
                REQUIRE(v == 0);    // DEBT: Probably this should be undefined, but I know it will be zero here
            }
        }
        SECTION("complex iterator")
        {
            SECTION("unusual delimiter")
            {
                // TBD
                const char* in = "123/";
            }
            SECTION("istreambuf_iterator")
            {
                typedef estd::layer1::stringbuf<32> streambuf_type;
                streambuf_type sb = test::str_uint1;
                typedef estd::experimental::istreambuf_iterator<streambuf_type> iterator_type;
                iterator_type it(sb), end;
                experimental::num_get<char, iterator_type> n;

                auto result = n.get(it, end, state, holder, v);

                REQUIRE(holder.good());
                REQUIRE(v == test::uint1);
            }
        }
    }
    SECTION("use_facet")
    {
        SECTION("ctype")
        {
            constexpr char c = 'a';
            char result = experimental::use_facet<experimental::ctype<char> >(l).widen(c);
            REQUIRE(result == c);
        }
    }
}