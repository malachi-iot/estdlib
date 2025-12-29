#pragma once

#include "base.h"
#include "si.h"

// NOTE: This is more of a reference implementation thank anything.
// at this time we expect actual unit implementations to occur in consuming libraries
// (embr, j1939).
// Note also this is an oddball reference because unlike miles, volts, etc.:
// 1.  a fraction of a byte doesn't always make sense and when it does, it has unique rules for naming (nibble, bit, etc)
// 2.  unlike all other units, 1024 is the kilo step not 1000

namespace estd { namespace units { inline namespace v1 {

struct bytes_tag {};

template <class Rep, class Period = estd::ratio<1>, typename F = passthrough<Rep> >
using bytes = unit<Rep, Period, bytes_tag, F>;

template <class Rep, typename F = passthrough<Rep> >
using kilobytes = bytes<Rep, estd::ratio<1024>, F>;

template <class Rep, typename F = passthrough<Rep> >
using megabytes = bytes<Rep, estd::ratio<1024 * 1024>, F>;

template <class Rep, typename F = passthrough<Rep> >
using gigabytes = bytes<Rep, estd::ratio<1024 * 1024 * 1024>, F>;

}}}

namespace estd { namespace internal { namespace units {

using bytes_tag = estd::units::v1::bytes_tag;

template <class Rep, class Period = estd::ratio<1>, typename F = passthrough<Rep> >
using bytes = estd::units::v1::bytes<Rep, Period, F>;

template <>
struct traits<bytes_tag>
{
    static constexpr const char* name() { return "bytes"; }
    static constexpr const char* abbrev() { return "B"; }
};

namespace si {

// specialize how we determine what a kilobyte, megabyte, etc. string gets computed as

template <>
struct traits<estd::ratio<1024>, bytes_tag> : traits<estd::kilo>
{
};

template <>
struct traits<estd::ratio<1024 * 1024>, bytes_tag> : traits<estd::mega>
{
};

template <>
struct traits<estd::ratio<1024 * 1024 * 1024>, bytes_tag> : traits<estd::giga>
{
};

}

}}}


namespace estd { inline namespace literals { inline namespace units_literals {

constexpr units::v1::bytes<unsigned> operator ""_bytes (unsigned long long int v)
{
    return units::v1::bytes<unsigned>(v);
}

constexpr units::v1::bytes<double> operator ""_bytes (long double v)
{
    return units::v1::bytes<double>(static_cast<double>(v));
}

}}}
