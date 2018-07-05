#include <catch.hpp>

#include <estd/port/chrono.h>

TEST_CASE("chrono tests")
{
    SECTION("basic 1 second test")
    {
        estd::chrono::seconds sec(1);
        estd::chrono::milliseconds ms(sec);
        estd::chrono::duration<uint32_t, estd::deci> de = ms;

        REQUIRE(ms.count() == 1000);
        REQUIRE(de.count() == 10);
    }
    SECTION("minutely tests")
    {
        estd::chrono::minutes minutes(2);
        estd::chrono::seconds sec = minutes;

        REQUIRE(sec.count() == 120);
    }
    SECTION("Ratio dividing")
    {
        SECTION("Simplest case")
        {
            typedef estd::ratio<1, 3> one_third;
            typedef estd::ratio<1, 6> one_sixth;
            typedef estd::ratio_divide<one_third, one_sixth> r;

            CONSTEXPR int num = r::num;
            CONSTEXPR int den = r::den;

            REQUIRE(num == 6);
            REQUIRE(den == 3);
        }
        SECTION("std lib example case")
        {
            typedef estd::ratio<2, 3> two_third;
            typedef estd::ratio<1, 6> one_sixth;
            typedef estd::ratio_divide<two_third, one_sixth> r;

            CONSTEXPR int num = r::num;
            CONSTEXPR int den = r::den;

            REQUIRE(num == 4);
            REQUIRE(den == 1);
        }
    }
}
