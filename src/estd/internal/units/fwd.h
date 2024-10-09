#pragma once

#include "concepts.h"
#include "../macros.h"

// Copying the playbook from chrono

namespace estd { namespace internal { namespace units {

template <class>
struct traits;

template <typename Int, Int sub>
struct subtractor;

template <typename Int, Int add>
struct adder;

struct unit_base_tag {};

template <class T>
struct passthrough
{
    using value_type = T;

    // DEBT: Sloppy, but less sloppy than slapping negative signs everywhere
    using reversal = passthrough<T>;

    constexpr T operator()(T v) const { return v; }

#if __cpp_constexpr >= 201304L   // "relaxed constexpr" (just to make debugging easier)
    // Just for diagnostic for more visibility
    template <class T2>
    constexpr T operator()(const T2& v) const
    {
        const T converted(v);
        return converted;
    }
#endif
};


template <class Rep, class Period, class Tag,
    ESTD_CPP_CONCEPT(Projector<Rep>) = passthrough<Rep> >
class unit_base;

template <class Rep, class Period>
struct cast_helper;

template <class, class>
struct compound_tag;

namespace si {

// DEBT: Do a concept here to filter by ratio
template <class Period, class Tag = void>
struct traits;

}

inline namespace literals {}


}}}
