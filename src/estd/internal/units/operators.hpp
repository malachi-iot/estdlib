#pragma once

#include <estd/ratio.h>

#include "fwd.h"

namespace estd { namespace internal { namespace units {

// DEBT: Consolidate with all the clever chrono "common_type" versions - for now
// implementing only the most basic to avoid too much code duplication
// TODO: Needs to get smarter and account for mismatched Rep, Period and F

template <typename Rep, class Period, class Tag, class F>
constexpr unit_base<Rep, Period, Tag, F> operator +(
    const unit_base<Rep, Period, Tag, F>& lhs,
    const unit_base<Rep, Period, Tag, F>& rhs)
{
    return unit_base<Rep, Period, Tag, F>{ (Rep) (lhs.root_count() + rhs.root_count()) };
}

template <typename Rep, class Period, class Tag, class F>
constexpr unit_base<Rep, Period, Tag, F> operator -(
    const unit_base<Rep, Period, Tag, F>& lhs,
    const unit_base<Rep, Period, Tag, F>& rhs)
{
    return unit_base<Rep, Period, Tag, F>{ Rep(lhs.root_count() - rhs.root_count()) };
}

/*
 * TODO: Not quite ready yet, need a specialization of common_type which can wrangle
 * unit_base similar to the chrono::duration variety
template <class Rep1, class Period1, class Rep2, class Period2, class Tag>
constexpr bool operator==(const unit_base<Rep1, Period1, Tag>& lhs,
    const unit_base<Rep2, Period2, Tag>& rhs)
{
    typedef typename estd::common_type<unit_base<Rep1, Period1, Tag>,
        unit_base<Rep2, Period2, Tag> >::type CT;

    return CT(lhs).count() == CT(rhs).count();
}
*/


}}}