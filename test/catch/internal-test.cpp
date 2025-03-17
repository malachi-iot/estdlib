#include <catch2/catch.hpp>

#include "estd/internal/value_evaporator.h"
#include "estd/internal/chooser.h"
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


template <class TEval>
struct Evaporator2 : estd::internal::struct_evaporator<TEval>
{
    typedef estd::internal::struct_evaporator<TEval> base_type;

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
                SECTION("basic")
                {
                    // inheritance is where the optimization magic happens
                    Evaporator value;

                    // Is only the size of Evaporator's "placeholder"
                    REQUIRE(sizeof(value) == sizeof(uint16_t));

                    REQUIRE(value.value().val1 == 5);
                }
                SECTION("template type infused")
                {
                    SECTION("empty base")
                    {
                        Evaporator2<EmptyClass> ev2;
                        typedef decltype(ev2)::base_type::evaporated_type reference;

                        REQUIRE(ev2.is_evaporated);
                        REQUIRE(sizeof(ev2) == sizeof(uint16_t));

                        //reference r = ev2.value();
                        REQUIRE(estd::is_const<reference>::value);
                        REQUIRE(!estd::is_reference<reference>::value);
                    }
                    SECTION("actual base")
                    {
                        Evaporator2<Dummy> ev2;
                        typedef decltype(ev2)::base_type::evaporated_type reference;

                        REQUIRE(!ev2.is_evaporated);
                        // Not doing == because padding can bulk it up
                        REQUIRE(sizeof(ev2) > sizeof(uint16_t) + sizeof(Dummy));

                        reference r = ev2.value();
                        r.val1 = 5;
                        REQUIRE(ev2.value().val1 == 5);
                        REQUIRE(estd::is_reference<reference>::value);
                    }
                }
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
    SECTION("chooser")
    {
        estd::internal::chooser c;

        const char* values[] { "zero", "one", "two" };
        const char* input = "one";

        SECTION("iterated")
        {
            REQUIRE(!c.process(values, *input++));
            REQUIRE(!c.process(values, *input++));
            REQUIRE(c.process(values, *input++));

            REQUIRE(c.chosen() == 1);
        }
        SECTION("normal")
        {
            c.choose(values, input, input + 3, false);

            REQUIRE(c.chosen() == 1);
        }
        SECTION("incomplete")
        {
            c.choose(values, input, input + 2, false);

            REQUIRE(c.chosen() == -1);
        }
        SECTION("static")
        {
            const char* input = "two";
            REQUIRE(estd::internal::chooser::choose(values, input) == 2);
        }
    }
}
