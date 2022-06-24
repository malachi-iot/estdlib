#include <catch.hpp>

#include <estd/ratio.h>

TEST_CASE("ratio")
{
    SECTION("math")
    {
        typedef estd::ratio<1, 3> one_third;
        typedef estd::ratio<2, 3> two_third;
        typedef estd::ratio<1, 6> one_sixth;
        typedef estd::ratio<25, 100> quarter;

        SECTION("dividing")
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
        SECTION("multiplying")
        {
            SECTION("2/3 * 1/6")
            {
                typedef estd::ratio_multiply<two_third, one_sixth> r;

                CONSTEXPR int num = r::num;
                CONSTEXPR int den = r::den;

                REQUIRE(num == 1);
                REQUIRE(den == 9);
            }
            SECTION("25/100 * 2")
            {
                typedef estd::ratio_multiply<quarter, estd::ratio<2> > r;

                CONSTEXPR int num = r::num;
                CONSTEXPR int den = r::den;

                REQUIRE(num == 1);
                REQUIRE(den == 2);
            }
        }
        SECTION("adding")
        {
            typedef estd::ratio_add<two_third, one_sixth> r;

            CONSTEXPR int num = r::num;
            CONSTEXPR int den = r::den;

            REQUIRE(num == 5);
            REQUIRE(den == 6);
        }
        SECTION("reduced")
        {
            typedef quarter::type reduced;

            CONSTEXPR int num = reduced::num;
            CONSTEXPR int den = reduced::den;

            REQUIRE(num == 1);
            REQUIRE(den == 4);
        }
    }
}