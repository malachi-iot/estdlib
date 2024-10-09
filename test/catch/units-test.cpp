#include <catch.hpp>

#include <estd/internal/units/base.h>
#include <estd/internal/units/bytes.h>
#include <estd/internal/units/ostream.h>
#include <estd/internal/units/percent.h>
#include <estd/internal/units/operators.hpp>
#include <estd/sstream.h>
#include <estd/type_traits.h>

struct frequency_tag {};

template <class Rep, class Period = estd::ratio<1>, typename F = estd::internal::units::passthrough<Rep> >
using hz = estd::internal::units::unit_base<Rep, Period, frequency_tag, F>;


namespace estd { namespace internal { namespace units {

template <>
struct traits<frequency_tag>
{
    static constexpr const char* name() { return "hertz"; }
    static constexpr const char* abbrev() { return "Hz"; }
};

}}}

TEST_CASE("units")
{
    using namespace estd::internal::units;

    estd::layer1::ostringstream<256> out;
    auto& s = out.rdbuf()->str();

    SECTION("hz")
    {
        hz<double> v(0);

        REQUIRE(v.count() == 0);

        out << estd::put_unit(v);

        REQUIRE(s == "0.00Hz");
    }
    SECTION("bytes")
    {
        bytes<unsigned> v(2048);
        kilobytes<double> v2(v);

        out << estd::put_unit(v);

        REQUIRE(s == "2048B");

        out.rdbuf()->clear();

        out << estd::put_unit(v2);

        REQUIRE(s == "2.00kB");
    }
    SECTION("operators")
    {
        // DEBT: 55_pct doesn't auto play nice with percent<double>

        percent<int16_t, estd::ratio<100, 1024> >
            adc_p1(512), adc_p2(100);
        percent<double> p1(55.0_pct);
        //percent<float> p2(45.0_pct);
        percent<float> p2(45.0);

        SECTION("addition")
        {
            // would need conversion too, we're not there yet
            //auto v = p1 + p2;
            auto v = adc_p1 + adc_p2;

            REQUIRE(v.count() == 612);
        }
        SECTION("subtraction")
        {
            auto v = adc_p2 - adc_p1;

            REQUIRE(v.count() == -412);
        }
        SECTION("negation")
        {
            auto v = -adc_p1;

            REQUIRE(v.count() == -512);
        }
        SECTION("greater than")
        {
            const percent<uint32_t> p3{90};

            // common_type doesn't like mixing floats and ints. that's fair
            //bool v = p1 > adc_p2;
            bool v = p3 > adc_p1;

            REQUIRE(v);
        }
    }
    SECTION("ostream")
    {
        //percent<double> p = 50_pct;   // FIX: It considers this narrowing, but wouldn't unsigned -> double be the opposite?
        percent<double> p = 50.0_pct;

        SECTION("regular")
        {
            out << put_unit(p);

            REQUIRE(s == "50.00%");
        }
        SECTION("non abbrev")
        {
            out << put_unit(p, false);

            REQUIRE(s == "50.00 percent");
        }
    }
    SECTION("percent")
    {
        percent<uint16_t, estd::ratio<1, 10> > percent1{974};

        REQUIRE(percent<uint8_t>(percent1).count() == 97);

        auto percent2 = 50_pct;

        REQUIRE(percent2.count() == 50);

        percent<double> percent3{50.1};

        percent1 = percent3;

        REQUIRE(percent1.count() == 501);

        percent1.root_count(507);
        percent3 = percent1;

        REQUIRE(percent3.count() == 50.7);

        REQUIRE(percent3 == 50.7_pct);
        bool comp = percent1 == percent3;
        REQUIRE(comp);

        // lhs precision is lower so dangling .7% falls away
        REQUIRE(50_pct == percent1);

        SECTION("ostream percent2")
        {
            write_abbrev(out, percent2);

            REQUIRE(out.rdbuf()->str() == "50%");
        }
        SECTION("ostream percent3")
        {
            write(out, percent3);

            REQUIRE(s == "50.70 percent");
        }
        SECTION("conversion")
        {
            SECTION("common type")
            {
                percent<uint8_t, estd::ratio<1, 10>> p1{0};
                percent<int32_t> p2{0};

                using CT = decltype(common_type_helper(p1, p2));
                static_assert(estd::is_same<CT::period, estd::ratio<1, 10>>::value, "");
                static_assert(estd::is_same<CT::rep, int32_t>::value, "");
                //period v1;

                CT p3{p2};

                REQUIRE(p3.count() == 0);
            }
            SECTION("int <--> float")
            {
                // Need 100:1024 because ->
                // 512/1024 = 0.5 then we need * 100
                percent<int16_t, estd::ratio<100, 1024> > adc1{512};
                percent<int> p{adc1};
                percent<double> percent4{adc1};

                REQUIRE(p.count() == 50);

                percent3 = adc1;

                REQUIRE(percent3.count() == 50);

                adc1 = percent1;

                REQUIRE(adc1.count() == 519);
            }
        }
    }
}


TEST_CASE("units (outside namespace)")
{
    //namespace units = estd::internal::units;

    estd::internal::units::percent<int16_t, estd::ratio<100, 1024> >
        p1(512), p2(100);

    auto p3 = p1 + p2;

    REQUIRE(p3.count() == 612);
}
