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
        // DEBT: Don't use auto
        auto error_code1 = estd::make_error_code(errc::invalid_argument);
        string_view s = error_code1.message();

        REQUIRE(s == "invalid_argument");

        REQUIRE(
            error_code1.default_error_condition().value() ==
            EINVAL);

        REQUIRE(
            error_code1.value() ==
            EINVAL);
    }
    SECTION("error_condition")
    {
        // DEBT: Don't use auto
        auto error_code1 = estd::make_error_code(errc::invalid_argument);
        REQUIRE(
            estd::make_error_condition(errc::invalid_argument).value() ==
            EINVAL);

        auto error_cond1 = internal::system_category::default_error_condition(error_code1.value());
        REQUIRE(error_cond1.value() == EINVAL);
    }
}

#include "macro/pop.h"
