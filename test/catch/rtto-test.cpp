#include <catch2/catch_all.hpp>

#include <estd/internal/rtto.h>

#include "test-data.h"

#include "macro/push.h"

using namespace estd;

TEST_CASE("rtto", "Runtime Type Operations")
{
    using util = internal::rtto_base::utility_type;

    SECTION("test::Dummy")
    {
        using rtto = internal::rtto<test::Dummy>;
        const util u = rtto::utility;
        int destruct_counter = 0;

        test::Dummy d1(1, "Dummy1", &destruct_counter);
        test::Dummy d2, d3;

        move_to(u, &d1, &d2);

        REQUIRE(d1.moved_from_);
        REQUIRE(d2.moved_);

        REQUIRE(destruct_counter == 0);

        REQUIRE(move_to_and_destroy(u, &d2, &d3) == 0);

        // FIX: Not working right
        //REQUIRE(destruct_counter == 1);
    }
}

#include "macro/pop.h"
