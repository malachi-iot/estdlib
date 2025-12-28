#pragma once

#include "../../ratio.h"

#include "../fwd/limits.h"
#include "fwd.h"

namespace estd { namespace internal { namespace units {


// EXPERIMENTAL
template <class Unit>
struct unit_traits_old
{
    using value_type = typename Unit::rep;
    using limits = estd::numeric_limits<value_type>;

    static constexpr value_type min()
    {
        return limits::min();
    }

    static constexpr value_type max()
    {
        return limits::max();
    }
};


// 'traits' occupied by just tag.  Might consider shifting names around at some point
// tag_traits?
template <class Rep, class Period, class Tag,
    ESTD_CPP_CONCEPT(Projector<Rep>) F>
struct unit_traits
    //: traits<Tag> // DEBT: Bring this guy back, temporarily disabling while I sort out dependencies
{
    using rep = Rep;
    using period = Period;
    using tag = Tag;
    using projector = F;
};


template <class Rep, class Tag>
struct basic_unit_traits : unit_traits<Rep, estd::ratio<1>, Tag, passthrough<Rep>> {};


}}}
