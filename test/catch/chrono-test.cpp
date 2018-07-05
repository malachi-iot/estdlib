#include <catch.hpp>

// TODO: Once we get POSIX mode for this, include in estd/chrono.h directly
#include <estd/port/chrono.h>

class fake_clock
{
public:
    int ticks;

    typedef estd::internal::miilli_rep rep;
    typedef estd::ratio<1, 15> period; // each 'fake' tick is 1/15 of a second
    typedef estd::chrono::duration<rep, period> duration;
    typedef estd::chrono::time_point<fake_clock> time_point;

    time_point now()
    {
        return time_point(duration(ticks));
    }
};

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
    SECTION("fake_clock tests")
    {
        fake_clock clock;

        clock.ticks = 0;

        auto first = clock.now();

        clock.ticks += 30; // 2 seconds for this fake_clock

        auto second = clock.now();

        // first and second are 'time_points' (absolute time) and in the native fake_clock
        // tick format.  the subtraction converts to a duration (relative time) and then
        // duration_cast converts to milliseconds.  count() merely accesses the underlying
        // resulting millisecond count
        auto count = estd::chrono::duration_cast<estd::chrono::milliseconds>(second - first).count();

        REQUIRE(count == 2000);

        clock.ticks += 5; // = 1/3 of a second

        count = estd::chrono::duration_cast<estd::chrono::milliseconds>(clock.now() - second).count();

        REQUIRE(count == 333);

        // needs this https://en.cppreference.com/w/cpp/chrono/duration/common_type specialization
        // which in turns needs greatest common divider for the two ratios
        //bool result = (clock.now() - second) > estd::chrono::milliseconds(500);
    }
}
