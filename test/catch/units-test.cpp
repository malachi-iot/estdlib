#include <catch2/catch_all.hpp>

#include <estd/units.h>

#if FEATURE_STD_CHRONO
#include <chrono>
#endif

#include <estd/internal/units/mixins.h>

#include <estd/sstream.h>
#include <estd/type_traits.h>

struct frequency_tag {};

template <class Rep, class Period, class F>
struct frequency_unit_traits : estd::units::v1::detail::traits<Rep, Period, frequency_tag, F>
{
    static constexpr auto options =
        estd::units::detail::default_initialized |
        estd::units::detail::permissive_1_1;
};

template <class Rep, class Period, class Tag>
struct value_init_unit_traits : estd::units::v1::detail::traits<Rep, Period, Tag>
{
    static constexpr auto options =
        estd::units::detail::options::value_initialized |
        estd::units::detail::permissive_1_1;

    constexpr static Rep default_value() { return 7; }
};

template <class Rep, class Period, class Tag>
struct permissive_unit_traits : estd::units::v1::detail::traits<Rep, Period, Tag>
{
    static constexpr auto options = estd::units::detail::options::permissive;
};


template <class Rep, class Period = estd::ratio<1>, typename F = estd::units::passthrough<Rep>>
using hz = estd::units::v1::detail::unit<frequency_unit_traits<Rep, Period, F>>;

template <class Rep, class Period = estd::ratio<1>, typename F = estd::units::passthrough<Rep>>
using permissive_hz = estd::units::v1::detail::unit<permissive_unit_traits<Rep, Period, frequency_tag>>;

namespace estd { namespace internal { namespace units {

// DEBT: Last holdout, still needs to live in estd::internal::units
template <>
struct traits<frequency_tag>
{
    static constexpr const char* name() { return "hertz"; }
    static constexpr const char* abbrev() { return "Hz"; }
};

}}}

// TODO: Put this and some other things in a 'synthetic' or other consumable test area,
// because this and things like 'Dummy' are just useful for other libs too
namespace Catch {

using namespace estd::units::v1;

template <class Rep, class Period, class Tag, class F>
struct StringMaker<unit<Rep, Period, Tag, F>>
{
    using human_type = unit<double, estd::ratio<1>, Tag, F>;

    static std::string convert(unit<Rep, Period, Tag, F> const& v)
    {
        // DEBT: Perhaps put this code into some diagnostic/explicit to_string.
        // i.e. we somewhat duplicated this in embr::mem

        std::ostringstream oss;

        if(Period::num != Period::den)
            oss << put_unit(human_type(v, relaxed_narrow_t{})) << " (count=" << v.count() << ")";
        else
            oss << put_unit(v);

        return oss.str();
    }
};
}

TEST_CASE("units")
{
    using namespace estd::units::v1;
    using namespace estd::literals::units_literals;

    estd::layer1::ostringstream<256> out;
    auto& s = out.rdbuf()->str();

#if FEATURE_ESTD_OSTREAM_FLOAT
    out.precision(2);
#elif ESTD_OSTREAM_DEFAULT_PRECISION != 2
#warning "Precision does not appear to be set or settable to 2, as needed"
#endif

    REQUIRE(out.precision() == 2);

    SECTION("make_ostream_like")
    {
        std::ostringstream oss;
        estd::internal::make_ostream_like(oss);
    }
    SECTION("prerequisites")
    {
        SECTION("1:1")
        {
            using type = percent<int>;
            using traits2 = detail::traits<long, estd::ratio<1>, type::tag>;

            static_assert(treat_as_floating_point<int, type::tag>::value == false);
            static_assert(type::can_unit_convert<traits2>());
        }
        SECTION("100:1024 (25:256)")
        {
            using type = percent<int16_t, estd::ratio<100, 1024> >;
            using traits2 = detail::traits<uint32_t, estd::ratio<1>, type::tag>;

            static_assert(treat_as_floating_point<int, type::tag>::value == false);
            static_assert(!type::can_unit_convert<traits2>());
        }
    }
    SECTION("hz")
    {
        SECTION("basics")
        {
            hz<double> v(0);

            REQUIRE(v.count() == 0);

#if FEATURE_STD_CHARCONV
            out << put_unit(v);

            REQUIRE(s == "0.00Hz");
#endif
        }
        SECTION("unassigned sanity check")
        {
            // DEBT: GCC and clang type pun for us, but others won't
            union
            {
                hz<int> hz1;
                int raw1{123};
            };

            static_assert(std::is_trivial<hz<int>>::value);

            REQUIRE(hz1.count() == 123);
        }
    }
    SECTION("bytes")
    {
        bytes<unsigned> v(2048);
        estd::units::v1::kilobytes<double> v2(v);

        out << put_unit(v);

        REQUIRE(s == "2048B");

        out.rdbuf()->clear();

#if FEATURE_STD_CHARCONV
        out << put_unit(v2);

        REQUIRE(s == "2.00kB");
#endif
    }
    SECTION("operators")
    {
        // DEBT: 55_pct doesn't auto play nice with percent<double>

        using percent_type = estd::units::v1::percent<int16_t, estd::ratio<100, 1024> >;
        percent_type adc_p1(512), adc_p2(100);
        percent<double> p1(55.0_pct);
        //percent<float> p2(45.0_pct);
        percent<float> p2(45.0);
        const percent<uint32_t> p3{90};
        percent<int> p4{50};

        struct special_traits : estd::units::detail::basic_traits<int, estd::internal::units::percent_tag>{};

        detail::unit<special_traits> p5{1};
        percent<int16_t> p6{1};

        using options = detail::options;

        // DEBT: Unweildy way of adding 'permissive' to options
        using permissive_traits =
            estd::units::detail::rebindable_traits<int16_t, estd::ratio<1>,
            special_traits::tag, special_traits::projector,
            options(options::permissive_period | options::default_initialized)>;

        detail::unit<permissive_traits> p7{10};

        SECTION("addition")
        {
            auto v = adc_p1 + adc_p2;

            REQUIRE(v.count() == 612);

            auto v2 = p3 + adc_p2;

            // 1:1024 precision
            REQUIRE(v2.count() == 102160);

            // almost there, some fine detail about converting float <--> double having an issue
            //auto v3 = p1 + p2;

            // implicit precision loss not permitted here
            //adc_p2 += p3;
            adc_p2 += unit_cast<percent_type>(p3);

            REQUIRE(adc_p2.count() == 1021);

            // ratio<1> specializations

            p1 += 3;

            REQUIRE(p1 == 58);

            // compatible types

            p1 += p2;

            REQUIRE(p1 == 103);

            p1 += p6;

            REQUIRE(p1 == 104.0_pct);

            p1 += p5;

            REQUIRE(p1 == 105.0_pct);

            //p5 += p1;   // Doesn't compile because p1 is double and p5 is int
            p5 += p4;
            //p5 += p3;     // Doesn't compile because p3 uint32_t doesn't pass narrowing conversion to int

            // DEBT: Make Stringizer more resilient to basic_traits and friends
            REQUIRE(p5.count() == 51);
        }
        SECTION("addition: precision change during +/-")
        {
            SECTION("percent: 8-bit")
            {
                percent<int8_t> p1(-50);
                percent<uint8_t> p2(50);

                p2 -= p1;

                REQUIRE(p2 == 100);
            }
            SECTION("bytes: 16-bit")
            {
                bytes<int16_t> b1(20000);
                bytes<uint16_t> b2(20000);

                b2 += b1;

                REQUIRE(b2 == 40000);

                // Disallowed due to potential precision loss
                //b1 -= b2;
            }
        }
        SECTION("subtraction")
        {
            p4 -= uint8_t(5);
            p4 -= int64_t(5);

            auto v = adc_p2 - adc_p1;

            REQUIRE(v.count() == -412);
            REQUIRE(p4 == 40);
        }
        SECTION("negation")
        {
            auto v = -adc_p1;

            REQUIRE(v.count() == -512);
        }
        SECTION("greater than")
        {
            // common_type doesn't like mixing floats and ints. that's fair
            //bool v = p1 > adc_p2;
            bool v = p3 > adc_p1;

            REQUIRE(v);
        }
        SECTION("multiply")
        {
            p2 *= 2;
            p1 = p2 * 3;

            //detail::unit_put(p1);

            // DEBT: Add ability to compare against narrowed 90_pct etc
            REQUIRE(p2 == 90.0_pct);
            REQUIRE(p1 == 270.0_pct);

            p4 = p4 * 2;
        }
        SECTION("multiply: precision change")
        {
            constexpr int16_t m1 = 4;

            // NOTE: p7 has a more permissive implicit constructor, but that's not actually
            // needed to test multiply precision change since that is permissive already.
            // Specifically, since C++ doesn't complain if we do:
            // int16 m; m = m + m1;
            // Then we follow suit
            p7 = p7 * m1;

            p6 = p6 * m1;
        }
        SECTION("divide")
        {
            p2 /= 2;
            p1 = p2 / 3;

            REQUIRE(p2 == 22.5_pct);
            REQUIRE(p1 == 7.5_pct);
        }
        SECTION("other permissive tests")
        {
            percent<int16_t> v(5.0);

            v += 50;

            REQUIRE(v == 55);

            // It's OK to precision loss, as demonstrated by chrono below.
            // We're supposed to kick back if periods mismatch in such a way where
            // the ratio division yields a precision loss (Rep is largely ignored)
            percent<int32_t> v2(v);
            percent<int16_t> v3(v2);

            REQUIRE(v3 == v);

#if FEATURE_STD_CHRONO
            std::chrono::duration<int32_t> d1(5);
            std::chrono::duration<int16_t> d2(d1);

            REQUIRE(d2.count() == 5);
#endif
        }
    }
#if FEATURE_STD_CHARCONV
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
#endif
    SECTION("percent")
    {
        percent<uint16_t, estd::ratio<1, 10> > percent1{974};

        REQUIRE(unit_cast<percent<uint8_t>>(percent1) == 97);

        auto percent2 = 50_pct;

        REQUIRE(percent2.count() == 50);

        percent<double> percent3{50.1};

        percent1 = unit_cast<decltype(percent1)>(percent3);

        REQUIRE(percent1.count() == 501);

        percent1.root_count(507);
        percent3 = percent1;

        REQUIRE(percent3.count() == 50.7);

        REQUIRE(percent3 == 50.7_pct);
        // DEBT: Consider auto promotion to double type from int.  Undecided at this time
        // whether I want to allow it
        bool comp = percent<double>(percent1) == percent3;
        REQUIRE(comp);

        // == / != is smart enough to auto promote 50_pct to percent1 type, so it knows
        // these don't match, despite potential precision loss
        REQUIRE(50_pct != percent1);

        SECTION("ostream percent2")
        {
            write_abbrev(make_ostream_like(out), percent2);

            REQUIRE(out.rdbuf()->str() == "50%");
        }
#if FEATURE_STD_CHARCONV
        SECTION("ostream percent3")
        {
            write(make_ostream_like(out), percent3);

            REQUIRE(s == "50.70 percent");
        }
#endif
#if EXPOSITIONAL_ONLY
        SECTION("prohibited constructor")
        {
            // Correctly fails to compile, units default behavior is to demand initialization
            percent<int> p;
        }
#endif
        SECTION("initialization")
        {
            SECTION("value-init")
            {
                using hz = estd::units::detail::unit<
                    value_init_unit_traits<int, estd::ratio<1>, frequency_tag>>;

                hz v;

                // 1:1 specializations help us here
                REQUIRE(v == 7);
            }
            SECTION("default-init")
            {
                union
                {
                    int v1{10};
                    hz<int> v2;
                };

                REQUIRE(v2 == 10);
            }
        }
        SECTION("conversion")
        {
            SECTION("common_type")
            {
                SECTION("test1")
                {
                    percent<uint8_t, estd::ratio<1, 10>> p1{0};
                    percent<int32_t> p2{0};

                    using CT = decltype(ct_helper(p1, p2));
                    static_assert(estd::is_same<CT::period, estd::ratio<1, 10>>::value, "");    // NOLINT
                    static_assert(estd::is_same<CT::rep, int32_t>::value, "");                  // NOLINT
                    //period v1;

                    CT p3{p2};

                    REQUIRE(p3.count() == 0);
                }
                SECTION("test2")
                {
                    percent<uint16_t, estd::ratio<100, 1024>> p1{0};
                    percent<uint32_t> p2{0};

                    using CT = decltype(ct_helper(p1, p2));

                    static_assert(estd::is_same<CT::rep, uint32_t>::value, ""); // NOLINT
                    // NOTE: since 100:1024 precision has some clicky fine points that a regular
                    // 1:1 integer wouldn't, we promote to 1:1024
                    static_assert(estd::is_same<CT::period, estd::ratio<1, 1024>>::value, "");  // NOLINT
                }
                // Does indeed fail to compile, as intended
#if EXPOSITIONAL_ONLY
                SECTION("mismatch tags")
                {
                    hz<uint16_t> hz1(10);
                    percent<uint16_t> p1(10);

                    // static assert - mismatched types
                    using CT = decltype(ct_helper(hz1, p1));
                }
#endif
                SECTION("permissive")
                {
                    detail::unit<permissive_unit_traits<int32_t, estd::ratio<1, 8>, frequency_tag>> u1(0);
                    detail::unit<permissive_unit_traits<int16_t, estd::ratio<1>, frequency_tag>> u2(u1);
                    permissive_hz<int16_t> u3(u1);

                    SECTION("permissive and regular mixed")
                    {
                        hz<int> h1(1);
                        auto v1 = u3 - h1;
                        REQUIRE(v1.count() == -1);
                    }
                }
            }
            SECTION("int <--> float")
            {
                // Need 100:1024 because ->
                // 512/1024 = 0.5 then we need * 100
                percent<int16_t, estd::ratio<100, 1024> > adc1{512};
                percent<int> p{adc1, relaxed_narrow_t{}};
                percent<double> percent4{adc1};

                REQUIRE(p.count() == 50);

                percent3 = adc1;

                REQUIRE(percent3.count() == 50);

                adc1 = unit_cast<decltype(adc1)>(percent1);

                REQUIRE(adc1.count() == 519);
            }
            SECTION("same tag, different traits")
            {
                using hz_init = estd::units::detail::unit<
                    value_init_unit_traits<int, estd::ratio<1>, frequency_tag>>;

                hz_init v1;
                hz<int> v2(v1);

                REQUIRE(v2 == v1);

                v1 = v2 + 1;

                REQUIRE(v1 == 8);
            }
            SECTION("rebind")
            {
                using type1 = percent<double>;
                using traits1 = typename type1::traits;
                using traits2 = typename traits1::rebind<int>;
                using type2 = estd::units::detail::unit<traits2>;

                static_assert(std::is_same<traits2::rep, int>::value);
                static_assert(std::is_same<type2::rep, int>::value);
            }
            SECTION("implicit convert to rep")
            {
                // Correctly disallowed
                //int val = hz<int>(0);

                int val = permissive_hz<int>(5);

                REQUIRE(val == 5);
            }
        }
    }
}


TEST_CASE("units (outside namespace)")
{
    //namespace units = estd::internal::units;

    estd::units::percent<int16_t, estd::ratio<100, 1024> >
        p1(512), p2(100);

    auto p3 = p1 + p2;

    REQUIRE(p3.count() == 612);
}
