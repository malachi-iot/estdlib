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

using namespace estd;


template <class T>
static expected<T, errc> err_return()
{
#if __cpp_deduction_guides
    return unexpected(errc::invalid_argument);
#else
    return unexpected<errc>(errc::invalid_argument);
#endif
}

static void handler(expected<int, errc> e)
{

}

typedef estd::test::NonTrivial ExplicitError;

TEST_CASE("expected")
{
    SECTION("int value_type")
    {
        typedef estd::expected<int, estd::errc> expected_type;

        SECTION("default")
        {
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
        SECTION("assignment operator")
        {
            expected_type e;

            e = 10;

            REQUIRE(e.has_value());
            REQUIRE(*e == 10);

            e = unexpected<errc>(errc::invalid_argument);

            REQUIRE(!e.has_value());
            REQUIRE(e.error() == errc::invalid_argument);
        }
        SECTION("non trivial error type")
        {
            typedef estd::expected<int, ExplicitError> expected_type2;

            expected_type2 e;

            REQUIRE(e.has_value());
            REQUIRE(*e == 0);
        }
        SECTION("explicit converting E")
        {
            typedef estd::expected<int, const char*> expected_type2;

            //expected_type e(estd::errc::invalid_argument);    // Beware, this cascades out to non-error value
            expected_type e2(estd::unexpected<int>(0));

            REQUIRE(e2.has_value() == false);
            REQUIRE(e2.error() == 0);

            //expected_type2 e3("hello");
            expected_type2 e4(estd::unexpected<const char*>("hello"));
        }
        SECTION("implicit conversion to E")
        {
            REQUIRE(err_return<int>().error() == estd::errc::invalid_argument);
        }
        SECTION("copy constructor")
        {
            expected_type e1;

            e1 = unexpected<errc>(errc::invalid_argument);

            // FIX: This seems to be doing trivial copy constructor
            expected_type e2(e1);

            REQUIRE(e2.has_value() == false);

            e2 = 7;

            REQUIRE(e2.has_value() == true);
        }
    }
    SECTION("void value_type")
    {
        typedef estd::expected<void, estd::errc> expected_type;

        SECTION("non-error")
        {
            expected_type e;

            REQUIRE(e.has_value());
        }
        SECTION("with error")
        {
            expected_type e(estd::unexpect_t{}, estd::errc::invalid_argument);

            REQUIRE(!e.has_value());
            REQUIRE(e.error() == estd::errc::invalid_argument);
        }
    }
    SECTION("non-trivial value_type")
    {
        typedef estd::expected<test::NonTrivial, int> expected_type;

        SECTION("with value")
        {
            expected_type e(estd::in_place_t{}, 5);

            REQUIRE(e.has_value());
            REQUIRE(e.value().code_ == 5);
        }
        SECTION("with error")
        {
            expected_type e(estd::unexpect_t{}, 4);

            REQUIRE(!e.has_value());
            REQUIRE(e.error() == 4);
        }
        SECTION("copy constructor")
        {
            expected_type e(in_place_t{}, 5);
            expected_type e2(e);

            REQUIRE(e2.has_value());
            REQUIRE(e2.value().code_ == 5);

            //REQUIRE(test::NonTrivial::dtor_counter == test::dtor_count_1());
        }
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

        handler(unexpected<errc>(errc::bad_address));
    }
    SECTION("pointer type")
    {
        test::NonTrivial nt(10);
        using type = expected<test::NonTrivial*, unsigned>;
        type e(&nt);

        REQUIRE(e.has_value());
    }
    // DEBT: Put this elsewhere
    SECTION("variant")
    {
        SECTION("largest_type")
        {
            constexpr int v = sizeof(estd::internal::largest_type<ExplicitError, char>::type);

            REQUIRE(v == sizeof(ExplicitError));
        }
        SECTION("are_trivial")
        {
            // TODO: Move these out to proper algorithm/type_traits area
            REQUIRE(estd::is_trivial<void>::value == false);    // DEBT: Unclear to me what to expect from void here
            REQUIRE(estd::is_trivial<ExplicitError>::value == false);

            REQUIRE(estd::internal::are_trivial<int, int>::value);
            REQUIRE(estd::internal::are_trivial<int, ExplicitError>::value == false);
            REQUIRE(estd::internal::are_trivial<void, int>::value == false);
        }
    }
}
