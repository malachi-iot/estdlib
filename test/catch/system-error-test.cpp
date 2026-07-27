#include <estd/system_error.h>

#include <catch2/catch_all.hpp>

#include "macro/push.h"

using namespace estd;

TEST_CASE("system_error tests")
{
    SECTION("errc")
    {
        errc error(estd::errc::invalid_argument);

        REQUIRE(error == errc::invalid_argument);
        REQUIRE(error != errc::result_out_of_range);

        REQUIRE(error == estd::errc(EINVAL));
    }
    SECTION("error_code")
    {
        string_view s = estd::make_error_code(errc::invalid_argument).message();

        REQUIRE(s == "invalid_argument");

        REQUIRE(
            estd::make_error_code(errc::invalid_argument).default_error_condition().value() ==
            EINVAL);

        REQUIRE(
            estd::make_error_code(errc::invalid_argument).value() ==
            EINVAL);

        REQUIRE(
            estd::make_error_condition(errc::invalid_argument).value() ==
            EINVAL);

    }
}

#include "macro/pop.h"
