#include <catch.hpp>

#include <estd/chrono.h>
#include <estd/limits.h>

using namespace estd;

TEST_CASE("limits & common_type tests")
{
    // https://stackoverflow.com/questions/15211463/why-isnt-common-typelong-unsigned-longtype-long-long
    // summarizes for us that common_type will not go to something like an int64_t - instead it is going
    // to choose one of the two
    SECTION("common_type")
    {
        SECTION("signed test 1")
        {
            typedef typename common_type<int16_t, uint32_t>::type common_type;

            //int sz = sizeof(common_type);

            //REQUIRE(sz == 4);
            //REQUIRE((numeric_limits<common_type>::digits) == 32);
            //auto max = numeric_limits<common_type>::max();
            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 32);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be unsigned
            REQUIRE(!is_signed);
        }
        SECTION("signed test 2")
        {
            typedef typename common_type<uint16_t, int32_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 31);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(is_signed);
        }
        SECTION("signed test 3")
        {
            typedef typename common_type<int32_t, uint32_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 32);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(!is_signed);
        }
        SECTION("signed test 4")
        {
            typedef typename common_type<int8_t, uint8_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits >= 15);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(is_signed);
        }
        SECTION("signed test 5")
        {
            typedef typename common_type<int16_t, uint8_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            // NOTE: common_type loves to promote bitness, even when
            // not strictly necessary
            REQUIRE(digits >= 15);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(is_signed);
        }
        SECTION("signed test 6")
        {
            typedef typename common_type<int64_t, uint8_t>::type common_type;

            auto digits = numeric_limits<common_type>::digits;
            REQUIRE(digits == 63);
            auto is_signed = numeric_limits<common_type>::is_signed;
            // because of aforementioned comment, this should be signed
            REQUIRE(is_signed);
        }
        SECTION("internal::cond_t")
        {
            typedef decltype (std::declval<int16_t>()) t1;
            typedef decltype (std::declval<uint8_t>()) t2;
            // It seems that this statement always results in a 32-bit type
            typedef decltype (false ? std::declval<int16_t>() : std::declval<uint8_t>()) t3;

            int digits;

            // Interestingly we have to decay t1 and t2 as they come out to be &&
            digits = numeric_limits<estd::decay_t<t1> >::digits;

            REQUIRE(digits == 15);

            digits = numeric_limits<estd::decay_t<t2> >::digits;

            REQUIRE(digits == 8);

            digits = numeric_limits<estd::decay_t<t3> >::digits;

            // NOTE: common_type's love of big bitness boils down to
            // the t3-represented ternary behavior.  Likely related
            // to native machine word preferences
            REQUIRE(digits >= 15);

            typedef internal::cond_t<int16_t, uint8_t> cond_type;

            digits = numeric_limits<cond_type>::digits;

            REQUIRE(digits >= 15);
        }
    }
    SECTION("bool")
    {
        REQUIRE(numeric_limits<bool>::max());
    }
    SECTION("8 bit")
    {
        auto digits = numeric_limits<int8_t>::digits;

        REQUIRE(digits == 7);

        digits = numeric_limits<uint8_t>::digits;

        REQUIRE(digits == 8);
    }
    SECTION("16 bit")
    {
        auto digits = numeric_limits<int16_t>::digits;

        REQUIRE(digits == 15);

        digits = numeric_limits<uint16_t>::digits;

        REQUIRE(digits == 16);
    }
    SECTION("32 bit")
    {
        auto digits = numeric_limits<int32_t>::digits;

        REQUIRE(digits == 31);

        digits = numeric_limits<uint32_t>::digits;

        REQUIRE(digits == 32);
    }
    SECTION("promote_type")
    {
        SECTION("1:1")
        {
            typedef typename promoted_type<uint8_t, uint8_t>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 8);
        }
        SECTION("simple")
        {
            typedef typename promoted_type<int8_t, int16_t>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 15);
        }
        SECTION("uint8_t -> int16_t")
        {
            typedef typename promoted_type<uint8_t, int16_t>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 15);
        }
        SECTION("int8_t -> uint8_t (auto promote to int16_t)")
        {
            // We promote to int16_t by default to avoid precision loss
            typedef typename promoted_type<uint8_t, int8_t>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 15);
        }
        SECTION("int16_t -> uint16_t (auto promote to int32_t)")
        {
            // We promote to int32_t by default to avoid precision loss
            typedef typename promoted_type<uint16_t, int16_t>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 31);
        }
        SECTION("int16_t -> uint16_t (permit 'promoting' to same bitness int16_t)")
        {
            // We shut off auto-promote for this, indicating we are confident
            // no precision loss going from uint16_t -> int16_t
            typedef typename promoted_type<uint16_t, int16_t, false>::type type;

            auto digits = numeric_limits<type>::digits;

            REQUIRE(digits == 15);
        }
    }
}
