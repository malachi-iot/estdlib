#include <catch.hpp>

#include "estd/internal/value_evaporator.h"
#include "test-data.h"

using namespace estd::test;

struct EvaporatorBase
{
    uint16_t val1;

    EvaporatorBase() : val1{5} {}
    EvaporatorBase(uint16_t val1) : val1{val1} {}
};

struct Evaporator : estd::internal::struct_evaporator<EvaporatorBase, false>
{
    uint16_t placeholder;
};

TEST_CASE("Internal tests", "[internal]")
{
    SECTION("value_evaporator")
    {

    }
    SECTION("struct_evaporator")
    {
        SECTION("evaporated")
        {
            SECTION("direct")
            {
                // non-inherited always occupy one byte for empty structs
                estd::internal::struct_evaporator<EvaporatorBase, false> value;

                REQUIRE(sizeof(value) == 1);

                REQUIRE(value.value().val1 == 5);
            }
            SECTION("inherited")
            {
                // inheritance is where the optimization magic happens
                Evaporator value;

                // Is only the size of Evaporator's "placeholder"
                REQUIRE(sizeof(value) == sizeof(uint16_t));

                REQUIRE(value.value().val1 == 5);
            }
        }
        SECTION("present")
        {
            SECTION("direct")
            {
                estd::internal::struct_evaporator<EvaporatorBase, true> value(EvaporatorBase{3});

                REQUIRE(sizeof(value) == sizeof(EvaporatorBase));
                REQUIRE(value.value().val1 == 3);
            }
        }
    }
}
