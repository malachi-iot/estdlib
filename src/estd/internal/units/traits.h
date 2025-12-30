#pragma once

#include "../../flags.h"
#include "../../ratio.h"

#include "fwd.h"

namespace estd { namespace units { inline namespace v1 {

namespace detail {

enum options
{
    none,

    default_prohibited,
    value_initialized,
    default_initialized
};

// 'traits' occupied by just tag.  Might consider shifting names around at some point
// tag_traits?
template <class Rep, class Period, class Tag,
    ESTD_CPP_CONCEPT(Projector<Rep>) F>
struct traits
    //: traits<Tag> // DEBT: Bring this guy back, temporarily disabling while I sort out dependencies
{
    using rep = Rep;
    using period = Period;
    using tag = Tag;
    using projector = F;

    static constexpr detail::options options = detail::options::default_prohibited;

    constexpr static rep default_value() { return {}; }
};


template <class Rep, class Tag>
struct basic_traits : traits<Rep, ratio<1>, Tag, passthrough<Rep>> {};

} // detail

}}} // estd::units::inline v1

namespace estd { namespace internal { namespace units {

using options = ::estd::units::v1::detail::options;

}}}
