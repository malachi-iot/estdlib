#pragma once

#include "../../flags.h"
#include "../../ratio.h"

#include "fwd.h"

namespace estd { namespace internal { namespace units {

enum options
{
    none,

    default_prohibited,
    default_value_initialized,
    default_unassigned
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

    static constexpr units::options options = units::options::default_prohibited;

    constexpr static rep default_value() { return {}; }
};


template <class Rep, class Tag>
struct basic_unit_traits : unit_traits<Rep, estd::ratio<1>, Tag, passthrough<Rep>> {};


}}}
