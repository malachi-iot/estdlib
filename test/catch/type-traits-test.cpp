#include <catch.hpp>

#include <estd/type_traits.h>
#include <estd/limits.h>
#include <estd/string.h>
#include <estd/internal/raw/variant.h>

#include "test-data.h"

using namespace estd;

#include "macro/push.h"

TEST_CASE("type traits tests")
{
    SECTION("make_signed")
    {
        typedef make_signed<uint16_t>::type should_be_int16_t;

        REQUIRE(sizeof(should_be_int16_t) == sizeof(int16_t));
        REQUIRE(is_signed<should_be_int16_t>::value);
        REQUIRE(!is_signed<uint16_t>::value);
    }
    SECTION("decay")
    {
        int digits = numeric_limits<decay_t<uint8_t> >::digits;
        REQUIRE(digits == 8);
        digits = numeric_limits<decay_t<int16_t> >::digits;
        REQUIRE(digits == 15);
        digits = numeric_limits<decay_t<uint16_t> >::digits;
        REQUIRE(digits == 16);
    }
    SECTION("remove_reference")
    {
        constexpr int value = 5;
        int _digit = value;
        int& digit = _digit;
        // removing reference means we are back to an int vs int&
        estd::remove_reference_t<decltype(digit)> digit_copy = digit;

        REQUIRE(digit_copy == _digit);

        digit_copy = value * 2;

        REQUIRE(_digit == value);
        REQUIRE(digit == value);
        REQUIRE(digit_copy == value * 2);
    }
    SECTION("char_traits")
    {
        SECTION("char")
        {
            REQUIRE(is_signed<char_traits<char>::off_type>::value);
            REQUIRE(!is_signed<char_traits<char>::pos_type>::value);
        }
        SECTION("const char")
        {
            REQUIRE(is_signed<char_traits<const char>::off_type>::value);
            REQUIRE(!is_signed<char_traits<const char>::pos_type>::value);
        }
        SECTION("uint8_t")
        {
            REQUIRE(is_signed<char_traits<uint8_t>::off_type>::value);
            REQUIRE(!is_signed<char_traits<uint8_t>::pos_type>::value);
        }
    }
    SECTION("is_base_of")
    {
        struct Parent {};

        SECTION("single inheritance")
        {
            struct Child : Parent {};

            REQUIRE(is_base_of<Parent, Child>::value);
        }
        SECTION("multiple inheritance")
        {
            struct Parent2 {};
            struct Parent3 {};

            struct Child : Parent, Parent2, Parent3 {};

            REQUIRE(is_base_of<Parent, Child>::value);
            REQUIRE(is_base_of<Parent2, Child>::value);
            REQUIRE(is_base_of<Parent3, Child>::value);
        }
    }
    SECTION("underlying_type")
    {
        enum test1 : int16_t { a, b, c };

        constexpr bool v = is_same<underlying_type<test1>::type, int16_t>::value;

        REQUIRE(v == true);
    }
    SECTION("is_convertible")
    {
        REQUIRE(is_convertible<test::ChildOfDummy, test::Dummy>::value == true);
        REQUIRE(is_convertible<test::Dummy, test::ChildOfDummy>::value == false);
        REQUIRE(is_convertible<test::ChildOfDummy*, test::Dummy*>::value == true);
        REQUIRE(is_convertible<test::Dummy*, test::ChildOfDummy*>::value == false);

        bool v1 = std::is_convertible<monostate, float>::value;

        REQUIRE(v1 == false);

        v1 = std::is_convertible<float, monostate>::value;

        REQUIRE(v1 == false);

        v1 = std::is_convertible<int, const char*>::value;

        REQUIRE(v1 == false);

        v1 = estd::is_constructible<const char*, int>::value;

        REQUIRE(v1 == false);

    }
}

#include "macro/pop.h"
