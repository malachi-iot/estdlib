#pragma once

#include "concepts.h"
#include "../macros.h"

// Copying the playbook from chrono

namespace estd { namespace internal { namespace units {

template <class Tag>
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
    ESTD_CPP_CONCEPT(Projector<Rep>) F = passthrough<Rep>>
struct unit_traits;

// 1:1 ratio, passthrough always - used for readability, to reduce error verbosity
template <class Rep, class Tag>
struct basic_unit_traits;

namespace v2 {

template <class Traits>
class unit_base;

}

inline namespace v1 {

template <typename Rep, class Period, class Tag,
    ESTD_CPP_CONCEPT(Projector<Rep>) F = passthrough<Rep>>
using unit_base = v2::unit_base<unit_traits<Rep, Period, Tag, F>>;

}


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
