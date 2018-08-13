//
// Created by malachi on 8/10/18.
//

#include <catch.hpp>

#include <estd/istream.h>
#include <estd/ostream.h>

TEST_CASE("iostreams")
{
    SECTION("SFINAE tests")
    {
        typedef estd::internal::impl::native_streambuf<char, estd::TEST_STREAM_T, std::char_traits<char> >
                streambuf_impl_type;
        bool has_method = estd::internal::has_sputc_method<streambuf_impl_type>::value;

        REQUIRE(has_method);
    }
    SECTION("cout")
    {
        //estd::basic_streambuf<char> streambuf(*stdout);
        estd::ostream _cout(*stdout);

        _cout << "Got here" << estd::endl;

        _cout.put('!');
    }
}
