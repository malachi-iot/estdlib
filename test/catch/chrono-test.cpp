#include <catch.hpp>

// TODO: Once we get POSIX mode for this, include in estd/chrono.h directly
#include <estd/chrono.h>

class fake_clock
{
public:
    int ticks;

    typedef estd::internal::milli_rep rep;
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
    SECTION("default time_point")
    {
        fake_clock::time_point default_time_point;
        fake_clock::duration epoch = default_time_point.time_since_epoch();

        REQUIRE(epoch.count() == 0);
        default_time_point += fake_clock::duration(100);
        REQUIRE(default_time_point.time_since_epoch().count() == epoch.count() + 100);
    }
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
    SECTION("daily tests")
    {
        estd::chrono::days days(5);
        estd::chrono::minutes minutes = days;

        REQUIRE(minutes.count() == (5 * 24 * 60));
    }
    SECTION("Ratios")
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
    SECTION("common_type specialization")
    {
#ifdef FEATURE_ESTD_CHRONO_EXP
        typedef estd::common_type<estd::chrono::seconds, estd::chrono::milliseconds>::type common;
        typedef common::period period;
        int num = period::num;
        int den = period::den;
        REQUIRE(num == 1);
        REQUIRE(den == 1000);

        typedef estd::common_type<estd::chrono::seconds, estd::chrono::minutes>::type common2;
        typedef common2::period period2;
        num = period2::num;
        den = period2::den;
        REQUIRE(num == 60);
        REQUIRE(den == 1);

        typedef estd::common_type<fake_clock::duration, estd::chrono::seconds>::type common3;
        typedef common3::period period3;
        num = period3::num;
        den = period3::den;
        REQUIRE(num == 1);
        REQUIRE(den == 15);

        SECTION("Equality")
        {
            fake_clock::duration lhs(3); // 3/15 = 1/5 of a second
            estd::chrono::milliseconds rhs(200); // 200ms = 1/5 of a second

            typedef typename estd::common_type<fake_clock::duration,
                                     estd::chrono::milliseconds>::type CT;

            num = CT::period::num;
            den = CT::period::den;

            // CT has to be a ratio which won't lose precision, so we're looking for something like
            // 1/1000 since it is the higher precision, however, it's not able to do exactly 1/3 of a second
            // where 1/15 does.  So we need a common type that doesn't lose precision during 1/1000
            // or 1/15 scenarios.  So, we can't have a denominator of < 1000, and it also has to be 3000
            // do divide evenly with 15.  So we actually want 1/3000 to represent ticks of either source

            REQUIRE(num == 1);
            REQUIRE(den == 3000);

            int count_lhs = CT(lhs).count();
            int count_rhs = CT(rhs).count();

            REQUIRE(count_lhs == count_rhs);
        }
#endif
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

#ifdef FEATURE_ESTD_CHRONO_EXP
        fake_clock::duration delta = clock.now() - second;
        // needs this https://en.cppreference.com/w/cpp/chrono/duration/common_type specialization
        // which in turns needs greatest common divider for the two ratios
        bool result = delta > estd::chrono::milliseconds(500);

        REQUIRE(!result);

        // pretty cool, 1/3 of a second really does fall between the 1/1000 so this is how
        // we have to compare
        result = delta > estd::chrono::milliseconds(333);

        REQUIRE(result);

        result = delta < estd::chrono::milliseconds(334);

        REQUIRE(result);
#endif

        second = first;

        REQUIRE(second == first);
    }
    SECTION("posix compat")
    {
        typedef estd::chrono::steady_clock clock;

        clock::time_point first = clock::now();
        clock::time_point next = clock::now();

        next += std::chrono::seconds(3);

        auto duration = next - first;

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        auto ms2 = estd::chrono::duration_cast<estd::chrono::milliseconds>(duration);

        REQUIRE(ms.count() == ms2.count());
    }
    SECTION("duration values")
    {
        REQUIRE(estd::chrono::duration_values<int>::max() == estd::numeric_limits<int>::max());
        REQUIRE(estd::chrono::duration_values<int16_t>::max() == estd::numeric_limits<int16_t>::max());

        REQUIRE(estd::chrono::duration<int>::min().count() == estd::numeric_limits<int>::min());
    }
    SECTION("comparisons")
    {
        estd::chrono::milliseconds mn(-5);
        estd::chrono::milliseconds mp(5);

        REQUIRE(mp > mn);

        REQUIRE(mn < mp);

        REQUIRE(mn <= mp);
        REQUIRE(mn <= mn);

        REQUIRE(abs(mp) == abs(mn));
    }
    SECTION("signed/unsigned test")
    {
        typedef estd::chrono::duration<uint8_t, estd::micro> microseconds_t;
        typedef estd::chrono::duration<int16_t, estd::micro> microseconds_st;

        microseconds_t value1(100), value2(105);

        microseconds_st value3 = value2 - value1;

        REQUIRE(value3.count() == 5);

        value3 = value1;
        value3 -= value2;

        REQUIRE(value3.count() == -5);

        // TODO: Does not yet compile
        //value3 = (microseconds_st)value1 - value2;
        // value3 = estd::chrono::duration_cast<microseconds_st>(value1) - value2;
        //value3 = value3 - value2;

        //REQUIRE(value3.count() == -5);
    }
}
