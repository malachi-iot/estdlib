#pragma once

#include "../raw/type_traits.h"

namespace estd { namespace units { inline namespace v1 {

template <class T>
struct passthrough
{
    using value_type = T;

    // DEBT: Sloppy, but less sloppy than slapping negative signs everywhere
    using reversal = passthrough<T>;

    // UNTESTED
    template <class T2>
    using rebind = passthrough<T2>;

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


}}}

namespace estd { namespace internal { namespace units {

template <typename Int, Int add>
struct adder;

// DEBT: Pretty sure there's a std/estd flavor of this we can use,
// though our flavor provides extra value in that reflected 'type'
// is important for consumer to use for precision/signing
template <typename Int, Int add>
struct subtractor : estd::integral_constant<Int, add>
{
    template <typename Int2>
    constexpr Int operator()(Int2 v) const
    {
        return v - add;
    }

    // DEBT: Sloppy, but less sloppy than slapping negative signs everywhere
    // else.
    using reversal = adder<Int, add>;
};

template <typename Int, Int add>
struct adder : estd::integral_constant<Int, add>
{
    template <typename Int2>
    constexpr Int operator()(Int2 v) const
    {
        return v + add;
    }

    // DEBT: Sloppy, but less sloppy than slapping negative signs everywhere
    // else.
    using reversal = subtractor<Int, add>;
};


}}}
