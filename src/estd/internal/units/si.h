#pragma once

#include <estd/ratio.h>
#include <estd/locale.h>

#include "fwd.h"

// NOTE: This looks like a job for 'locale', so we may end up putting it there

namespace estd { namespace internal { namespace units { namespace si {

template <>
struct traits<estd::kilo>
{
    static constexpr const char* name() { return "kilo"; }
    static constexpr const char* abbrev() { return "k"; }
};

template <>
struct traits<estd::mega>
{
    static constexpr const char* name() { return "mega"; }
    static constexpr const char* abbrev() { return "M"; }
};

template <>
struct traits<estd::giga>
{
    static constexpr const char* name() { return "giga"; }
    static constexpr const char* abbrev() { return "G"; }
};


template <>
struct traits<estd::deci>
{
    static constexpr const char* name() { return "deci"; }
    static constexpr const char* abbrev() { return "d"; }
};


template <>
struct traits<estd::deca>
{
    static constexpr const char* name() { return "deca"; }
    static constexpr const char* abbrev() { return "da"; }
};


template <>
struct traits<estd::milli>
{
    static constexpr const char* name() { return "milli"; }
    static constexpr const char* abbrev() { return "m"; }
};


}}}}