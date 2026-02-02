#pragma once

#include "../../ratio.h"

#include "fwd.h"
#include "common_type.h"

namespace estd { namespace units { inline namespace v1 { namespace detail {

template <class Traits>
constexpr unit<Traits> operator +(
    const unit<Traits>& lhs,
    const unit<Traits>& rhs)
{
    return unit<Traits>{ (typename Traits::rep) (lhs.root_count() + rhs.root_count()) };
}


template <class Traits1, class Traits2>
constexpr auto operator +(
    const unit<Traits1>& lhs,
    const unit<Traits2>& rhs) -> decltype(ct_helper(lhs, rhs))
{
    using CT = decltype(ct_helper(lhs, rhs));

    return CT(lhs) + CT(rhs);
}


template <class Traits>
constexpr unit<Traits> operator -(
    const unit<Traits>& lhs,
    const unit<Traits>& rhs)
{
    return unit<Traits>{ typename Traits::rep(lhs.root_count() - rhs.root_count()) };
}


template <class Traits1, class Traits2>
constexpr auto operator -(
    const unit<Traits1>& lhs,
    const unit<Traits2>& rhs) -> decltype(ct_helper(lhs, rhs))
{
    using CT = decltype(lhs + rhs);

    return CT(lhs) - CT(rhs);
}

// Comparisons

#define ESTD_UNITS_COMP(op) \
template <class Traits1, class Traits2> \
constexpr bool operator op(const unit<Traits1>& lhs, const unit<Traits2>& rhs) \
{   \
    using CT = decltype(lhs + rhs); \
    return CT(lhs).count() op CT(rhs).count(); \
}

ESTD_UNITS_COMP(>)
ESTD_UNITS_COMP(<)
ESTD_UNITS_COMP(>=)
ESTD_UNITS_COMP(<=)
ESTD_UNITS_COMP(==)
ESTD_UNITS_COMP(!=)

#undef ESTD_UNITS_COMP

// ratio<1> specializations

template <class Traits, typename Rep>
using compatible_rep = bool_constant<is_same<typename Traits::period, ratio<1>>::value && is_arithmetic<Rep>::value>;

template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr unit<Traits> operator+(unit<Traits>& lhs, const Rep& rhs)
{
    return lhs += unit<Traits>(rhs);
}


template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr unit<Traits> operator-(unit<Traits>& lhs, const Rep& rhs)
{
    return lhs -= unit<Traits>(rhs);
}

template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr unit<Traits>& operator +=(unit<Traits>& lhs, const Rep& rhs)
{
    return lhs += unit<Traits>(rhs);
}


template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr unit<Traits>& operator -=(unit<Traits>& lhs, const Rep& rhs)
{
    return lhs -= unit<Traits>(rhs);
}


template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator==(const unit<Traits>& lhs, const Rep& rhs)
{
    return lhs.count() == rhs;
}


template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator<(const unit<Traits>& lhs, const Rep& rhs)
{
    return lhs.count() < rhs;
}


template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator>(const unit<Traits>& lhs, const Rep& rhs)
{
    return lhs.count() > rhs;
}


template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator<=(const unit<Traits>& lhs, const Rep& rhs)
{
    return lhs.count() <= rhs;
}


template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator>=(const unit<Traits>& lhs, const Rep& rhs)
{
    return lhs.count() >= rhs;
}


template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator<(const Rep& lhs, const unit<Traits>& rhs)
{
    return lhs < rhs.count();
}

template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator<=(const Rep& lhs, const unit<Traits>& rhs)
{
    return lhs <= rhs.count();
}

template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator>(const Rep& lhs, const unit<Traits>& rhs)
{
    return lhs > rhs.count();
}

template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator>=(const Rep& lhs, const unit<Traits>& rhs)
{
    return lhs >= rhs.count();
}

template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator!=(const Rep& lhs, const unit<Traits>& rhs)
{
    return lhs != rhs.count();
}

template <class Traits, typename Rep,
    enable_if_t<compatible_rep<Traits, Rep>::value, int> = 0>
constexpr bool operator==(const Rep& lhs, const unit<Traits>& rhs)
{
    return lhs == rhs.count();
}

// regular arithmetic things

template <class Rep, class Traits,
    enable_if_t<is_arithmetic<Rep>::value, int> = 0>
constexpr auto operator *(
    unit<Traits> lhs,
    const Rep& rhs) -> decltype(ct_helper(lhs, rhs))
{
    return decltype(ct_helper(lhs, rhs)){ lhs.count() * rhs };
}

template <class Rep, class Traits,
    enable_if_t<is_arithmetic<Rep>::value, int> = 0>
constexpr auto operator /(
    unit<Traits> lhs,
    const Rep& rhs) -> decltype(ct_helper(lhs, rhs))
{
    return decltype(ct_helper(lhs, rhs)){ lhs.count() / rhs };
}

template <class Rep, class Traits,
    enable_if_t<is_arithmetic<Rep>::value, int> = 0>
constexpr auto operator %(
    unit<Traits> lhs,
    const Rep& rhs) -> decltype(ct_helper(lhs, rhs))
{
    return decltype(ct_helper(lhs, rhs)){ lhs.count() % rhs };
}


}}}}
