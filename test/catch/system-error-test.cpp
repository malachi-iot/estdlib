#include <estd/system_error.h>

#include <catch2/catch_all.hpp>

#include "macro/push.h"

TEST_CASE("system_error tests")
{
    SECTION("errc")
    {
        estd::errc error(estd::errc::invalid_argument);

        REQUIRE(error == errc::invalid_argument);
        REQUIRE(error != errc::result_out_of_range);

        REQUIRE(error == estd::errc(EINVAL));
    }
}

#include "macro/pop.h"
