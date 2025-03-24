#include <catch2/catch_all.hpp>

#include <estd/c++03/tuple.h>

#include "macro/push.h"

// Sometimes we have whole distinct c++03 code areas/files.  This is a catch-all unit test for that
TEST_CASE("c++03 sanity check")
{
    SECTION("tuple")
    {
        using namespace estd;

        struct fakeout
        {
            typedef int impl_type;
            impl_type impl;
        } f;

        tuple<float, float, char> v(2.0, 3.0, 'A');

        SECTION("low level get")
        {
            //internal::_iterate_test<1>(v.impl);
            //internal::get<1>(v.impl);
            auto r = internal::tuple_navigator<0, decltype(v.impl)>::value(v.impl);

            REQUIRE(r == 2);

            auto r2 = internal::tuple_navigator<2, decltype(v.impl)>::value(v.impl);

            REQUIRE(r2 == 'A');
        }
        SECTION("high level get")
        {
            auto r1 = get<0>(v);
            REQUIRE(r1 == 2.0);
            REQUIRE(get<1>(v) == 3.0);
            REQUIRE(get<2>(v) == 'A');

            // Both of the following are expected to generate compile time errors
            //REQUIRE(get<3>(v) == 'X');    // A succinct and kludgy but helpful error
            //REQUIRE(get<0>(f));           // A succinct, but not super helpful error
        }
        SECTION("low level set")
        {
            internal::tuple_navigator<1, decltype(v.impl)>::value(v.impl, 10);

            REQUIRE(get<1>(v) == 10);
        }
        SECTION("high level set")
        {
            set<1>(v, 10);
            REQUIRE(get<1>(v) == 10);
        }
        SECTION("const")
        {
            const tuple<float> v1{4};

            REQUIRE(get<0>(v1) == 4);
        }
    }
}

#include "macro/pop.h"
