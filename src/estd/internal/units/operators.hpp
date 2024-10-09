#pragma once

#include "../../ratio.h"

#include "fwd.h"
#include "common_type.h"

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

template <typename Rep1, class Period1, class Rep2, class Period2, class Tag, class Adder1, class Adder2>
constexpr auto operator +(
    const unit_base<Rep1, Period1, Tag, Adder1>& lhs,
    const unit_base<Rep2, Period2, Tag, Adder2>& rhs) -> decltype(ct_helper(lhs, rhs))
{
    using CT = decltype(ct_helper(lhs, rhs));

    return CT(lhs) + CT(rhs);
}

template <typename Rep, class Period, class Tag, class F>
constexpr unit_base<Rep, Period, Tag, F> operator -(
    const unit_base<Rep, Period, Tag, F>& lhs,
    const unit_base<Rep, Period, Tag, F>& rhs)
{
    return unit_base<Rep, Period, Tag, F>{ Rep(lhs.root_count() - rhs.root_count()) };
}


template <typename Rep1, class Period1, class Rep2, class Period2, class Tag, class Proj1, class Proj2>
constexpr auto operator -(
    const unit_base<Rep1, Period1, Tag, Proj1>& lhs,
    const unit_base<Rep2, Period2, Tag, Proj2>& rhs) -> decltype(ct_helper(lhs, rhs))
{
    using CT = decltype(lhs + rhs);

    return CT(lhs) - CT(rhs);
}



template <class Rep1, class Period1, class Rep2, class Period2, class Tag, class Adder1, class Adder2>
constexpr bool operator>(
    const unit_base<Rep1, Period1, Tag, Adder1>& lhs,
    const unit_base<Rep2, Period2, Tag, Adder2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() > CT(rhs).count();
}


template <class Rep1, class Period1, class Rep2, class Period2, class Tag, class Adder1, class Adder2>
constexpr bool operator<(
    const unit_base<Rep1, Period1, Tag, Adder1>& lhs,
    const unit_base<Rep2, Period2, Tag, Adder2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() < CT(rhs).count();
}


template <class Rep1, class Period1, class Rep2, class Period2, class Tag, class Adder1, class Adder2>
constexpr bool operator>=(
    const unit_base<Rep1, Period1, Tag, Adder1>& lhs,
    const unit_base<Rep2, Period2, Tag, Adder2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() >= CT(rhs).count();
}


template <class Rep1, class Period1, class Rep2, class Period2, class Tag, class Adder1, class Adder2>
constexpr bool operator<=(
    const unit_base<Rep1, Period1, Tag, Adder1>& lhs,
    const unit_base<Rep2, Period2, Tag, Adder2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() <= CT(rhs).count();
}



template <class Rep1, class Period1, class Rep2, class Period2, class Tag, class Adder1, class Adder2>
constexpr bool operator==(
    const unit_base<Rep1, Period1, Tag, Adder1>& lhs,
    const unit_base<Rep2, Period2, Tag, Adder2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() == CT(rhs).count();
}


template <class Rep1, class Period1, class Rep2, class Period2, class Tag, class Adder1, class Adder2>
constexpr bool operator!=(
    const unit_base<Rep1, Period1, Tag, Adder1>& lhs,
    const unit_base<Rep2, Period2, Tag, Adder2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() != CT(rhs).count();
}


}}}
