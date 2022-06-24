#include <catch.hpp>

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
        REQUIRE(num == 1);
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
        SECTION("inspect settled on common type")
        {
            typedef estd::chrono::duration<uint8_t, estd::micro> microseconds_t;
            typedef estd::chrono::duration<int16_t, estd::micro> microseconds_st;

            typedef estd::common_type<microseconds_t, microseconds_st>::type common;

            auto digits = estd::numeric_limits<common::rep>::digits;

            REQUIRE(digits == 15);
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

        bool should_be_true = mn <= mp;

        REQUIRE(should_be_true);
        REQUIRE(mn <= mn);

        REQUIRE(abs(mp) == abs(mn));

        // needs enhanced common_type to operate and switch between precisions
#ifdef FEATURE_ESTD_CHRONO_EXP
        SECTION("abs + bool")
        {
            typedef estd::chrono::duration<int8_t, estd::milli> milliseconds_st;

            milliseconds_st mp2(5);

            bool result = mn < mp2;

            REQUIRE(result);
        }
#endif
        SECTION("unsigned abs")
        {
            typedef estd::chrono::duration<uint16_t, estd::micro> microseconds_st;
            microseconds_st ms;

            // Does indeed not compile, as spec calls for
            // "Does not participate ... unless std::numeric_limits<Rep>::is_signed is true."
            //abs(ms);

            SECTION("internal")
            {
                // It's useful to have a noop flavor, so we do our own internal version of
                // abs which does just that
                estd::chrono::internal::abs(ms);
            }
        }
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

        // needs enhanced common_type to operate and switch between precisions
#ifdef FEATURE_ESTD_CHRONO_EXP
        //value3 = (microseconds_st)value1 - value2;    // actually works, but generates warning
        value3 = estd::chrono::duration_cast<microseconds_st>(value1) - value2;

        REQUIRE(value3.count() == -5);
#endif
    }
    SECTION("precision loss")
    {
        typedef estd::chrono::duration<int8_t, estd::milli> microseconds_t;
        typedef estd::chrono::duration<int16_t, estd::milli> microseconds_t2;

        microseconds_t t1(3);
        microseconds_t2 t2(6);

        t1 = t2;

        REQUIRE(t1 == t2);

        t2 += estd::chrono::duration_cast<microseconds_t2>(estd::chrono::seconds(2));

        REQUIRE(t2.count() == 2006);

        t1 = t2;

        // FIX: Here is the problem, silent precision loss
        //REQUIRE(t1 == t2);
    }
    SECTION("compile-time math and utility")
    {
        SECTION("gcd")
        {
            constexpr int v1 = estd::internal::gcd<24, 18>::value;
            REQUIRE(v1 == 6);
        }
        SECTION("lcm")
        {
            constexpr int v1 = estd::internal::lcm<4, 5>::value;

            REQUIRE(v1 == 20);
        }
        SECTION("chrono::common_type")
        {
            using namespace estd::chrono;

            SECTION("minutes and seconds")
            {
                typedef estd::common_type<minutes, seconds> _CT;

                REQUIRE((int) _CT::ratio_type::num == 1);
                REQUIRE((int) _CT::ratio_type::den == 1);
            }
            SECTION("microseconds and milliseconds")
            {
                typedef estd::common_type<microseconds, milliseconds> _CT;

                REQUIRE((int) _CT::ratio_type::num == 1);
                REQUIRE((int) _CT::ratio_type::den == 1000000);
            }
            SECTION("fake and hours")
            {
                typedef estd::common_type<fake_clock::duration, hours> _CT;

                REQUIRE((int) _CT::ratio_type::num == 1);
                REQUIRE((int) _CT::ratio_type::den == 15);
            }
            SECTION("oddball 1/7 and 4/3 times")
            {
                typedef duration<int, estd::ratio<1, 7> > d1;
                typedef duration<int, estd::ratio<4, 3> > d2;

                typedef estd::common_type<d1, d2> _CT;

                REQUIRE((int) _CT::ratio_type::num == 1);
                REQUIRE((int) _CT::ratio_type::den == 21);
            }
        }
    }
    SECTION("subtraction")
    {
        using namespace estd::chrono;

        seconds s(10);
        minutes m(2);

        auto d = m - s;

        REQUIRE(d.count() == 110);
    }
#if FEATURE_ESTD_CHRONO_LITERALS
    SECTION("literals")
    {
        using namespace estd::literals::chrono_literals;

        estd::chrono::seconds s = 10s + 4000ms + 1000000us;

        REQUIRE(s.count() == 15);
    }
#endif
    SECTION("C++20 style")
    {
        SECTION("year")
        {
            estd::chrono::year y{2001}, y2{4};

            auto y3 = y += y2;

            REQUIRE(y3 == 2005);
        }
        SECTION("year_month_day")
        {
            // DEBT: Can't use std::chrono::system_clock here because we are unable to convert
            // from estd::chrono::duration to std::chrono::duration
            // NOTE: We wouldn't want to grab now() anyway for unit test
            //estd::chrono::time_point<std::chrono::system_clock> tp{std::chrono::system_clock::now()};
            typedef fake_clock clock_type;

            SECTION("basic")
            {
                estd::chrono::time_point<clock_type> tp;
                estd::chrono::internal::year_month_day<clock_type> ymd{tp};

                REQUIRE(ymd.year() == 1970);
            }
            SECTION("specific")
            {
                // https://www.wikiwand.com/en/Unix_time
                estd::chrono::seconds dkuug_celebration{1000000000};
                estd::chrono::time_point<clock_type> tp{dkuug_celebration};
                estd::chrono::internal::year_month_day<clock_type> ymd{tp};

                auto year = ymd.year();

                REQUIRE(year == 2001);
                REQUIRE(ymd.month() == 9);

                // FIX: Not working, date/time calculations can get tricky
                //REQUIRE((int)ymd.days() == 9);
            }
        }
        SECTION("hh_mm_ss")
        {
            SECTION("basic")
            {
                estd::chrono::milliseconds s{3601005};
                estd::chrono::hh_mm_ss<decltype(s)> v{s};

                REQUIRE(v.hours().count() == 1);
                REQUIRE(v.minutes().count() == 0);
                REQUIRE(v.seconds().count() == 1);
                REQUIRE(v.subseconds().count() == 5);

                REQUIRE(v.is_negative() == false);
            }
            SECTION("unsigned")
            {
                typedef estd::chrono::duration<uint32_t, estd::milli> time_type;
                time_type ms{3601005};
                estd::chrono::hh_mm_ss<time_type> v{ms};

                auto hours = v.hours().count();

                REQUIRE(hours == 1);
                REQUIRE(v.minutes().count() == 0);
                REQUIRE(v.seconds().count() == 1);
                REQUIRE(v.subseconds().count() == 5);

                REQUIRE(v.is_negative() == false);
            }
            SECTION("negative")
            {
                estd::chrono::milliseconds s{-3601005};
                estd::chrono::hh_mm_ss<decltype(s)> v{s};

                REQUIRE(v.hours().count() == 1);
                REQUIRE(v.minutes().count() == 0);
                REQUIRE(v.seconds().count() == 1);
                REQUIRE(v.subseconds().count() == 5);

                REQUIRE(v.is_negative() == true);
            }
        }
    }
}
