#include <catch.hpp>

#include <estd/internal/units/base.h>
#include <estd/internal/units/bytes.h>
#include <estd/internal/units/ostream.h>
#include <estd/sstream.h>

struct frequency_tag {};

template <class Rep, class Period = estd::ratio<1>, typename F = estd::internal::units::passthrough<Rep> >
using hz = estd::internal::units::unit_base<Rep, Period, frequency_tag, F>;


namespace estd::internal::units {

template <>
struct traits<frequency_tag>
{
    static constexpr const char* name() { return "hertz"; }
    static constexpr const char* abbrev() { return "Hz"; }
};

}


TEST_CASE("units")
{
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
        estd::internal::units::bytes<unsigned> v(2048);
        estd::internal::units::kilobytes<double> v2(v);

        out << estd::put_unit(v);

        REQUIRE(s == "2048B");

        out.rdbuf()->clear();

        out << estd::put_unit(v2);

        REQUIRE(s == "2.00kB");
    }
}