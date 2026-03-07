#pragma once

namespace estd { namespace internal { namespace units {

template <class Tag1, class Tag2>
struct compound_tag
{
    using tag1_type = Tag1;
    using tag2_type = Tag2;
};

// EXPERIMENTAL
// Playing with a specialization-generated rate/compound unit (aka mph, kph, mAh, etc)
template <class TUnit1, class TUnit2,
    typename Rep = typename estd::promoted_type<
        typename TUnit1::rep,
        typename TUnit2::rep>::type >
struct compound_unit_helper
{
    using tag_type = compound_tag<
        typename TUnit1::tag_type,
        typename TUnit2::tag_type>;

    // FIX: Need to grab rep/period from somewhere.  Really this ought to be deduced
    // by looking at the lcd/gcd characteristics of both rep/period from both types
    using rep = Rep;
    using period = typename TUnit1::period;

    // TODO: Wrestle with f/offset-er later
    using type = estd::units::unit<rep, period, tag_type>;
};


}}}
