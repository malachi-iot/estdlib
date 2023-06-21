#include <catch.hpp>

#include <estd/optional.h>

#include "test-data.h"

using namespace estd;

estd::optional<int> returning_optional(int val)
{
    if(val == 10) return estd::nullopt;

    if(val == 20) return {};

    return val;
}

template <typename T, class TBase>
static void suite(estd::optional<T, TBase> o, T compare_to)
{
    REQUIRE(!o);
    bool result = o != T();
    REQUIRE(result);
    result = o == T();
    REQUIRE(!result);
    REQUIRE(!(o > compare_to));
    REQUIRE(!(o < compare_to));

    o = compare_to;

    result = o == compare_to;

    //REQUIRE(val > 4);
    REQUIRE(result);
}

enum SpecializedEnum
{
    Value1,
    Value2
};

enum PlainOldEnum
{
    POE_UNINITIALIZED = 0,
    POE_1,
    POE_2
};

// Any time SpecializedEnum is used in regular estd::optional, it routes to this base class
// See below unit test
namespace estd { namespace internal {

template<>
struct optional_base<SpecializedEnum> : optional_bitwise<SpecializedEnum, 4>
{
    typedef optional_bitwise<SpecializedEnum, 4> base_type;

    ESTD_CPP_FORWARDING_CTOR(optional_base)
};

}}


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

        REQUIRE(val.null_value() == -1);

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

            val = 3;

            val2 = val;

            REQUIRE(val2);

            int v = *val2;

            REQUIRE(v == 3);
        }
        SECTION("nullopt")
        {
            val = nullopt;

            REQUIRE(!val);

            layer1::optional<int, -1> val(nullopt);

            REQUIRE(!val);

            val = 5;

            optional<int> val2;

            // NOTE: according to spec, this is a semi-valid operation,
            // though leaning on the NO because we haven't set has_value to true this way
            //val2.value() = 1;     // Doesn't work due to bad_option_exception
            *val2 = 1;

            REQUIRE(!val2);

            val = val2;

            REQUIRE(!val);
        }
    }
    SECTION("comparisons")
    {
        SECTION("int")
        {
            estd::optional<int> val;

            suite(val, 4);

            val = 5;

            REQUIRE(val > 4);
            REQUIRE(val == 5);
        }
        SECTION("bool")
        {
            estd::optional<bool> val;

            suite(val, true);
        }
        SECTION("layer1: int")
        {
            estd::layer1::optional<int> val;

            suite(val, 4);

            val = 5;

            REQUIRE(val > 4);
            REQUIRE(val == 5);

            int val2 = -1;

            val = val2;

            REQUIRE(val);

            val2 = 0;
            val = val2;

            // layer1 flavor means 0 == null
            REQUIRE(!val);
        }
        SECTION("layer1: bool")
        {
            estd::layer1::optional<bool> val;

            suite(val, false);

            constexpr int sz = sizeof(val);

            REQUIRE(sz == sizeof(bool));
        }
        SECTION("layer1: enum")
        {
            estd::layer1::optional<PlainOldEnum, POE_UNINITIALIZED> o;

            o.reset();
        }
    }
    SECTION("function interaction/return value")
    {
        SECTION("standard")
        {
            estd::optional<int> value = returning_optional(5);

            REQUIRE(value);
            REQUIRE(*value == 5);

            value = returning_optional(10);

            REQUIRE(!value);
        }
        SECTION("layer1")
        {
            estd::layer1::optional<int, -1> value = returning_optional(5);

            REQUIRE(value);
            REQUIRE(value.value() == 5);

            value = returning_optional(10);

            REQUIRE(!value);
            REQUIRE(*value == -1);
        }
    }
    SECTION("auto specializing based on traits/base")
    {
        // This experiment works quite well, it's a matter of naming/convention/usability to work out now

        estd::optional<SpecializedEnum> o;

        REQUIRE(o.has_value() == false);

        o = Value1;

        REQUIRE(o == Value1);

        o.reset();
    }
    SECTION("non trivial")
    {
        estd::optional<test::NonTrivial> o;
        int counter = 0;

        REQUIRE(o.has_value() == false);

        o.emplace(7, [&]{ ++counter; });

        REQUIRE(o.has_value());
        REQUIRE(o->code_ == 7);
        REQUIRE(o->copied_ == false);
        REQUIRE(counter == 0);

        o.reset();

        REQUIRE(counter == 1);

        o.emplace(77, [&]{ ++counter; });

        SECTION("constructor")
        {
            optional<test::NonTrivial> o2(o), o_empty;
            optional<test::NonTrivial> o_empty2(o_empty);

            SECTION("move")
            {
                optional<test::NonTrivial> o3(std::move(o2));

                REQUIRE(o3->initialized_);
                REQUIRE(o3->copied_ == false);
                REQUIRE(o3->moved_);
            }

            REQUIRE(counter == 2);
        }
        SECTION("assignment")
        {
            optional<test::Dummy> o2(in_place_t{}, 7, "hi2u"), o3;

            // NonTrivial lacks assignment operator on purpose
            //optional<test::NonTrivial> o2;

            REQUIRE(!o2->copied_);
            REQUIRE(!o2->moved_);

            o3 = o2;

            REQUIRE(o3->copied_);
            REQUIRE(!o2->moved_);

            SECTION("move")
            {
                o2 = std::move(o3);

                REQUIRE(o2->moved_);
            }
        }
    }
}
