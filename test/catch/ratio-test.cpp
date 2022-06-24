#include <catch.hpp>

#include <estd/ratio.h>

TEST_CASE("ratio")
{
    SECTION("math")
    {
        typedef estd::ratio<1, 3> one_third;
        typedef estd::ratio<2, 3> two_third;
        typedef estd::ratio<1, 6> one_sixth;

        SECTION("Ratio dividing")
        {
            SECTION("Simplest case")
            {
                typedef estd::ratio_divide<one_third, one_sixth> r;

                CONSTEXPR int num = r::num;
                CONSTEXPR int den = r::den;

                REQUIRE(num == 6 / 3);
                REQUIRE(den == 3 / 3);
            }
            SECTION("std lib example case")
            {
                typedef estd::ratio_divide<two_third, one_sixth> r;

                CONSTEXPR int num = r::num;
                CONSTEXPR int den = r::den;

                REQUIRE(num == 4);
                REQUIRE(den == 1);
            }
        }
        SECTION("ratio multiplying")
        {
            typedef estd::ratio_multiply<two_third, one_sixth> r;

            CONSTEXPR int num = r::num;
            CONSTEXPR int den = r::den;

            REQUIRE(num == 1);
            REQUIRE(den == 9);
        }
    }
}