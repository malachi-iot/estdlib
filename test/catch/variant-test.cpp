#include <catch.hpp>

#include <estd/system_error.h>

#include <estd/internal/variant.h>

#include "test-data.h"

TEST_CASE("variant")
{
    SECTION("storage")
    {
        SECTION("monostate, int")
        {
            estd::internal::variant_storage2<estd::monostate, int> vs;

            REQUIRE(vs.is_trivial);
        }
        SECTION("int, int")
        {
            estd::internal::variant_storage2<int, int> vs;

            REQUIRE(vs.is_trivial);

            vs.storage.t1 = 7;

            auto& v = estd::internal::get<0>(vs);

            REQUIRE(v == 7);
        }
        SECTION("NonTrivial, int")
        {
            estd::internal::variant_storage2<estd::test::NonTrivial, int> vs;

            REQUIRE(!vs.is_trivial);
        }
    }
    SECTION("misc")
    {
        SECTION("index_of_type")
        {
            SECTION("basic")
            {
                typedef estd::internal::index_of_type<int, estd::monostate, int, float> iot;
                constexpr int idx = iot::index;

                REQUIRE(idx == 2);
            }
            SECTION("not found")
            {
                typedef estd::internal::index_of_type<int, estd::monostate, float> iot;

                constexpr int idx = iot::index;

                REQUIRE(idx == -1);
            }
        }
    }
}