#pragma once

#include "../../ratio.h"

#include "fwd.h"
#include "common_type.h"

namespace estd { namespace internal { namespace units {

// DEBT: Consolidate with all the clever chrono "common_type" versions - for now
// implementing only the most basic to avoid too much code duplication
// TODO: Needs to get smarter and account for mismatched Rep, Period and F

template <class Traits>
constexpr v2::unit_base<Traits> operator +(
    const v2::unit_base<Traits>& lhs,
    const v2::unit_base<Traits>& rhs)
{
    return v2::unit_base<Traits>{ (typename Traits::rep) (lhs.root_count() + rhs.root_count()) };
}

//template <typename Rep1, class Period1, class Rep2, class Period2, class Tag, class Adder1, class Adder2>
// FIX: Make sure tags match
template <class Traits1, class Traits2>
constexpr auto operator +(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs) -> decltype(ct_helper(lhs, rhs))
{
    using CT = decltype(ct_helper(lhs, rhs));

    return CT(lhs) + CT(rhs);
}

template <class Traits>
constexpr v2::unit_base<Traits> operator -(
    const v2::unit_base<Traits>& lhs,
    const v2::unit_base<Traits>& rhs)
{
    return v2::unit_base<Traits>{ typename Traits::rep(lhs.root_count() - rhs.root_count()) };
}


// FIX: match on Tag
template <class Traits1, class Traits2>
constexpr auto operator -(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs) -> decltype(ct_helper(lhs, rhs))
{
    using CT = decltype(lhs + rhs);

    return CT(lhs) - CT(rhs);
}


// TODO: Add operator* and operator-.  They work well enough BUT we may not want to return
// the same CT in those cases


// FIX: Match on tag
template <class Traits1, class Traits2>
constexpr bool operator>(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs)
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



// FIX: Match on tag
template <class Traits1, class Traits2>
constexpr bool operator==(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() == CT(rhs).count();
}


// FIX: Match on tag
template <class Traits1, class Traits2>
constexpr bool operator!=(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() != CT(rhs).count();
}

// ratio<1> specializations

template <class Traits, typename Rep,
    typename estd::enable_if_t<is_same<typename Traits::period, ratio<1>>::value && is_arithmetic<Rep>::value>* = nullptr>
constexpr v2::unit_base<Traits>& operator +=(
    v2::unit_base<Traits>& lhs,
    const Rep& rhs)
{
    return lhs += v2::unit_base<Traits>(rhs);
}


template <class Traits, typename Rep,
    typename estd::enable_if_t<is_same<typename Traits::period, ratio<1>>::value && is_arithmetic<Rep>::value>* = nullptr>
constexpr v2::unit_base<Traits>& operator -=(
    v2::unit_base<Traits>& lhs,
    const Rep& rhs)
{
    return lhs -= v2::unit_base<Traits>(rhs);
}


template <class Traits, typename Rep,
    typename estd::enable_if_t<is_same<typename Traits::period, ratio<1>>::value && is_arithmetic<Rep>::value>* = nullptr>
constexpr bool operator==(
    const v2::unit_base<Traits>& lhs,
    const Rep& rhs)
{
    return lhs.count() == rhs;
}

template <class Rep1, class Rep2, class Tag, class Proj,
    typename estd::enable_if_t<is_arithmetic<Rep2>::value>* = nullptr>
constexpr bool operator>(
    const unit_base<Rep1, estd::ratio<1>, Tag, Proj>& lhs,
    const Rep2& rhs)
{
    return lhs.count() > rhs;
}

template <class Rep1, class Rep2, class Tag, class Proj,
    typename estd::enable_if_t<is_arithmetic<Rep2>::value>* = nullptr>
constexpr bool operator<(
    const unit_base<Rep1, estd::ratio<1>, Tag, Proj>& lhs,
    const Rep2& rhs)
{
    return lhs.count() < rhs;
}


template <class Rep1, class Rep2, class Tag, class Proj,
    typename estd::enable_if_t<is_arithmetic<Rep2>::value>* = nullptr>
constexpr bool operator>=(
    const unit_base<Rep1, estd::ratio<1>, Tag, Proj>& lhs,
    const Rep2& rhs)
{
    return lhs.count() >= rhs;
}

template <class Rep1, class Rep2, class Tag, class Proj,
    typename estd::enable_if_t<is_arithmetic<Rep2>::value>* = nullptr>
constexpr bool operator<=(
    const unit_base<Rep1, estd::ratio<1>, Tag, Proj>& lhs,
    const Rep2& rhs)
{
    return lhs.count() <= rhs;
}

// regular arithmetic things

template <class Rep, class Traits,
    typename estd::enable_if_t<is_arithmetic<Rep>::value>* = nullptr>
constexpr auto operator *(
    v2::unit_base<Traits> lhs,
    const Rep& rhs) -> decltype(ct_helper(lhs, rhs))
{
    return decltype(ct_helper(lhs, rhs)){ lhs.count() * rhs };
}

template <class Rep, class Traits,
         typename estd::enable_if_t<is_arithmetic<Rep>::value>* = nullptr>
constexpr auto operator /(
    v2::unit_base<Traits> lhs,
    const Rep& rhs) -> decltype(ct_helper(lhs, rhs))
{
    return decltype(ct_helper(lhs, rhs)){ lhs.count() / rhs };
}

template <class Rep1, class Period, class Rep2, class Tag, class Proj,
    typename estd::enable_if_t<is_arithmetic<Rep2>::value>* = nullptr>
constexpr auto operator %(
    unit_base<Rep1, Period, Tag, Proj> lhs,
    const Rep2& rhs) -> decltype(ct_helper(lhs, rhs))
{
    return decltype(ct_helper(lhs, rhs)){ lhs.count() % rhs };
}


}}}
