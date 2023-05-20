#include <catch.hpp>

#include <estd/internal/variant.h>

#include "test-data.h"

using namespace estd;

TEST_CASE("variant")
{
    SECTION("main")
    {
        typedef internal::variant<int, test::NonTrivial> variant1_type;

        SECTION("default ctor")
        {
            variant1_type v;
            // Properly doesn't compile, since default value init goes for
            // first item
            //internal::variant<test::NonTrivial, int> v2;

            int& vref = get<int>(v);

            REQUIRE(v.index() == 0);
            REQUIRE(vref == 0);
            REQUIRE(internal::holds_alternative<int>(v));
        }
        SECTION("emplace")
        {
            variant1_type v;

            v.emplace<test::NonTrivial>(7);

            REQUIRE(v.index() == 1);
            REQUIRE(get<test::NonTrivial>(v).code_ == 7);

            REQUIRE(internal::get_if<0>(v) == nullptr);
            REQUIRE(internal::get_if<1>(v) != nullptr);
            REQUIRE(internal::get_if<1>(v)->code_ == 7);
            // FIX: NonTrivial test should be nullptr, since it's not at position 0
            REQUIRE(internal::get_if<int>(v) == nullptr);
            REQUIRE(internal::get_if<test::NonTrivial>(v) != nullptr);
        }
        SECTION("assign")
        {
            variant1_type v;

            v = 8;

            REQUIRE(get<int>(v) == 8);
        }
        SECTION("move")
        {
            variant1_type v;

            v = 8;

            variant1_type v2(std::move(v));
        }
    }
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
                constexpr bool multiple = iot::multiple;

                REQUIRE(idx == 1);
                REQUIRE(multiple == false);
            }
            SECTION("not found")
            {
                typedef estd::internal::index_of_type<int, estd::monostate, float> iot;

                constexpr int idx = iot::index;
                constexpr bool multiple = iot::multiple;

                REQUIRE(idx == -1);
                REQUIRE(multiple == false);
            }
            SECTION("multiple")
            {
                typedef estd::internal::index_of_type<int, int, int> iot;

                constexpr int idx = iot::index;
                constexpr bool multiple = iot::multiple;

                REQUIRE(idx == 0);
                REQUIRE(multiple == true);
            }
        }
    }
}
