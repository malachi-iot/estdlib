#pragma once

#include "../../flags.h"
#include "../../ratio.h"

#include "fwd.h"

namespace estd { namespace units { inline namespace v1 {

namespace detail {

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

    // DEBT: We ought to carry along default_value too
    // NOTE: 'unit' watches projector to determine rep
    template <class Rep2, class Period2 = Period, class F2 = typename projector::template rebind<Rep2>>
    using rebind = rebindable_traits<Rep2, Period2, Tag, F2, options>;
};


// for use cases which change rep and period around, gives us a way to retain options
// conveniently.  Not combining with above traits to reduce code error spew
template <class Rep, class Period, class Tag,
    ESTD_CPP_CONCEPT(Projector<Rep>) F, options o>
struct rebindable_traits : traits<Rep, Period, Tag, F>
{
    static constexpr detail::options options = o;

    template <class Rep2, class Period2 = Period, class F2 = F>
    using rebind = rebindable_traits<Rep2, Period2, Tag, F2, options>;
};

template <class Rep, class Tag>
struct basic_traits : traits<Rep, ratio<1>, Tag> {};

} // detail

}}} // estd::units::inline v1

namespace estd { namespace internal { namespace units {

using options = ::estd::units::v1::detail::options;

}}}
