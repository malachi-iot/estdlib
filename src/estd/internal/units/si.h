#pragma once

#include <estd/ratio.h>
#include <estd/locale.h>

#include "fwd.h"

// NOTE: This looks like a job for 'locale', so we may end up putting it there

namespace estd { namespace internal { namespace units { namespace si {

enum quantities
{
    time,
    length,
    mass,
    electric_current,
    temperature,
    amount_of_substance,
    luminous_intensity
};


template <class Tag>
struct traits<estd::kilo, Tag>
{
    static constexpr const char* name() { return "kilo"; }
    static constexpr const char* abbrev() { return "k"; }
};


template <class Tag>
struct traits<estd::mega, Tag>
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

// DEBT: Double check if this is actually an SI unit (I think it is)
template <class Tag>
struct traits<estd::tera, Tag>
{
    static constexpr const char* name() { return "tera"; }
    static constexpr const char* abbrev() { return "T"; }
};


template <class Tag>
struct traits<estd::deci, Tag>
{
    static constexpr const char* name() { return "deci"; }
    static constexpr const char* abbrev() { return "d"; }
};


template <class Tag>
struct traits<estd::deca, Tag>
{
    static constexpr const char* name() { return "deca"; }
    static constexpr const char* abbrev() { return "da"; }
};


template <class Tag>
struct traits<estd::centi, Tag>
{
    static constexpr const char* name() { return "centi"; }
    static constexpr const char* abbrev() { return "c"; }
};


template <class Tag>
struct traits<estd::milli, Tag>
{
    static constexpr const char* name() { return "milli"; }
    static constexpr const char* abbrev() { return "m"; }
};


template <>
struct traits<estd::micro>
{
    static constexpr const char* name() { return "micro"; }
    // TODO: When character set supports it, use the proper micro symbol here
    static constexpr const char* abbrev() { return "u"; }
};


template <>
struct traits<estd::nano>
{
    static constexpr const char* name() { return "nano"; }
    static constexpr const char* abbrev() { return "n"; }
};


}}}}
