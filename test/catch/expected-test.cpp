#include <catch.hpp>

#include <estd/system_error.h>

#include <estd/expected.h>

#include "test-data.h"

template <class T, class E>
void infuse_unexpected(E err)
{
    estd::unexpected<E> ue(err);
    estd::expected<T, E> e(ue);

    REQUIRE(e.error() == err);
    REQUIRE(e.error() == ue.error());
}

TEST_CASE("expected")
{
    SECTION("int value_type")
    {
        typedef estd::expected<int, estd::errc> expected_type;

        SECTION("default")
        {
            // FIX: this is supposed to be value initialized/default initialized -
            // but does that mean int should be 0?  Get to the bottom of this, because
            // it's not initializing at all at the moment (0 was only accidentally working
            // below)
            expected_type e;

            REQUIRE(e.has_value());
            REQUIRE(*e == 0);
        }
        SECTION("specific value initialized")
        {
            expected_type e(10);

            REQUIRE(e.has_value());
            REQUIRE(*e == 10);
        }
        SECTION("error state")
        {
            expected_type e(estd::unexpect_t{}, estd::errc::invalid_argument);

            REQUIRE(e.has_value() == false);
        }
    }
    SECTION("void value_type")
    {
        estd::expected<void, estd::errc> e;

        REQUIRE(e.has_value());
    }
    SECTION("Dummy (struct) type")
    {
        typedef estd::expected<estd::test::Dummy, estd::errc> expected_type;

        SECTION("default")
        {
            expected_type e;

            REQUIRE(e.has_value());
        }
        SECTION("specific constructor init")
        {
            expected_type e(estd::in_place_t{}, 1, "hello");

            REQUIRE(e.has_value());
        }
        SECTION("error type")
        {
            expected_type e(estd::unexpect_t{}, estd::errc::invalid_argument);

            REQUIRE(e.has_value() == false);
        }
    }
    SECTION("unexpected")
    {
        estd::unexpected<estd::errc> ue(estd::errc::invalid_argument);

        REQUIRE(ue.error() == estd::errc::invalid_argument);

        infuse_unexpected<int>(estd::errc::invalid_argument);
        infuse_unexpected<void>(estd::errc::invalid_argument);
        infuse_unexpected<estd::test::Dummy>(estd::errc::invalid_argument);
    }
}