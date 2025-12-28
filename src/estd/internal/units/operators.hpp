#pragma once

#include "../../ratio.h"

#include "fwd.h"
#include "common_type.h"

namespace estd { namespace internal { namespace units {

// DEBT: Consolidate with all the clever chrono "common_type" versions - for now
// implementing only the most basic to avoid too much code duplication

template <class Traits>
constexpr v2::unit_base<Traits> operator +(
    const v2::unit_base<Traits>& lhs,
    const v2::unit_base<Traits>& rhs)
{
    return v2::unit_base<Traits>{ (typename Traits::rep) (lhs.root_count() + rhs.root_count()) };
}

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


template <class Traits1, class Traits2>
constexpr bool operator>(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() > CT(rhs).count();
}


template <class Traits1, class Traits2>
constexpr bool operator<(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() < CT(rhs).count();
}


template <class Traits1, class Traits2>
constexpr bool operator>=(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() >= CT(rhs).count();
}


template <class Traits1, class Traits2>
constexpr bool operator<=(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() <= CT(rhs).count();
}



template <class Traits1, class Traits2>
constexpr bool operator==(
    const v2::unit_base<Traits1>& lhs,
    const v2::unit_base<Traits2>& rhs)
{
    using CT = decltype(lhs + rhs);

    return CT(lhs).count() == CT(rhs).count();
}


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

template <class Traits, typename Rep,
    typename estd::enable_if_t<is_same<typename Traits::period, ratio<1>>::value && is_arithmetic<Rep>::value>* = nullptr>
constexpr bool operator>(
    const v2::unit_base<Traits>& lhs,
    const Rep& rhs)
{
    return lhs.count() > rhs;
}

template <class Traits, typename Rep,
    typename estd::enable_if_t<is_same<typename Traits::period, ratio<1>>::value && is_arithmetic<Rep>::value>* = nullptr>
constexpr bool operator<(
    const v2::unit_base<Traits>& lhs,
    const Rep& rhs)
{
    return lhs.count() < rhs;
}


template <class Traits, typename Rep,
    typename estd::enable_if_t<is_same<typename Traits::period, ratio<1>>::value && is_arithmetic<Rep>::value>* = nullptr>
constexpr bool operator>=(
    const v2::unit_base<Traits>& lhs,
    const Rep& rhs)
{
    return lhs.count() >= rhs;
}

template <class Traits, typename Rep,
    typename estd::enable_if_t<is_same<typename Traits::period, ratio<1>>::value && is_arithmetic<Rep>::value>* = nullptr>
constexpr bool operator<=(
    const v2::unit_base<Traits>& lhs,
    const Rep& rhs)
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

template <class Rep, class Traits,
    typename estd::enable_if_t<is_arithmetic<Rep>::value>* = nullptr>
constexpr auto operator %(
    v2::unit_base<Traits> lhs,
    const Rep& rhs) -> decltype(ct_helper(lhs, rhs))
{
    return decltype(ct_helper(lhs, rhs)){ lhs.count() % rhs };
}


}}}
