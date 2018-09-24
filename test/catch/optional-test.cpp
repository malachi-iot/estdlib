#include <catch.hpp>

#include <estd/optional.h>

using namespace estd;

TEST_CASE("optional")
{
    SECTION("simple")
    {
        optional<int> val(5);
        //optional<int> val;

        //val = 5;

        REQUIRE(val.has_value());
    }
    SECTION("simple 2")
    {
        optional<int> val;

        REQUIRE(!val);

        val = 5;

        REQUIRE(val);
    }
    SECTION("null option")
    {
        optional<int> val(nullopt);

        REQUIRE(!val);

        val = 5;

        REQUIRE(val);
        REQUIRE(*val == 5);

        val = nullopt;

        REQUIRE(!val);
    }
    SECTION("looking for tag")
    {
        REQUIRE(internal::optional_tag_base::has_optional_tag_typedef<internal::optional_tag_base>::value);

        REQUIRE(optional<int>::has_optional_tag_typedef<internal::optional_tag_base>::value);
        REQUIRE(optional<int>::has_optional_tag_typedef<layer1::optional<int>>::value);

        REQUIRE(layer1::optional<int>::has_optional_tag_typedef<internal::optional_tag_base>::value);
        REQUIRE(layer1::optional<int>::has_optional_tag_typedef<optional<int>>::value);

        REQUIRE(!layer1::optional<int>::has_optional_tag_typedef<int>::value);
    }
    SECTION("layer1 version")
    {
        layer1::optional<int, -1> val;

        REQUIRE(*val == -1);

        REQUIRE(!val);

        val = 5;

        REQUIRE(val.has_value());

        int v = *val;

        REQUIRE(v == 5);

        SECTION("conversion to traditional")
        {
            optional<int> val2(val);
            layer1::optional<int> val3(val2);

            REQUIRE(val2);
            REQUIRE(val3);
            REQUIRE(*val2 == 5);
            REQUIRE(*val3 == 5);

            int sz = sizeof(val);

            REQUIRE(sz == sizeof(int));

            val2 = val;

            REQUIRE(val2);

            int v = *val2;

            REQUIRE(v == 5);
        }
        SECTION("nullopt")
        {
            val = nullopt;

            REQUIRE(!val);

            layer1::optional<int, -1> val(nullopt);

            REQUIRE(!val);
        }
    }
}
