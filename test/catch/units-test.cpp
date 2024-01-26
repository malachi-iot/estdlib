#include <catch.hpp>

#include <estd/internal/units/base.h>
#include <estd/internal/units/si.h>
//#include <estd/internal/units/ostream.h>

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
    hz<double> v(0);

    REQUIRE(v.count() == 0);
}