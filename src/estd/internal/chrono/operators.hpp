#pragma once

#include "../fwd/chrono.h"
#include "duration.hpp"
#include "ratio.h"
#include "../units/operators.hpp"
#include "../macro/push.h"

namespace estd { namespace chrono { namespace detail {

// NOTE: Not well tested for operations across different duration types
template <class Traits1, class Traits2>
estd::common_type_t<duration<Traits1>, duration<Traits2>>
constexpr operator-(const duration<Traits1>& lhs, const duration<Traits2>& rhs)
{
    using CT = common_type_t<duration<Traits1>, duration<Traits2>>;

    return CT(CT(lhs).count() - CT(rhs).count());
}



template <class Traits1, class Traits2>
estd::common_type_t<duration<Traits1>, duration<Traits2>>
constexpr operator+(const duration<Traits1>& lhs, const duration<Traits2>& rhs)
{
    using CT = common_type_t<duration<Traits1>, duration<Traits2>>;

    return CT(CT(lhs).count() + CT(rhs).count());
}

}

// In test where lhs is milliseconds and rhs is seconds, rhs doesn't auto promote to milliseconds correctly
#if NOT_READY
template<class Rep1, class Period1, class Rep2, class Period2>
common_type_t<duration<Rep1, Period1>, duration<Rep2, Period2>>
constexpr operator/(const duration<Rep1,Period1>& lhs,
    const duration<Rep2,Period2>& rhs )
{
    using CT = common_type_t<duration<Rep1, Period1>, duration<Rep2, Period2>>;

    return CT(CT(lhs).count() / CT(rhs).count());
}
#endif


template< class Rep1, class Period, class Rep2>
duration<typename estd::common_type<Rep1,Rep2>::type, Period>
constexpr operator*( const duration<Rep1,Period>& lhs,
    const Rep2& rhs )
{
    typedef duration<
        typename estd::common_type<Rep1,Rep2>::type, Period>
        CT;

    return CT(lhs.count() * rhs);
}


template< class Rep1, class Period, class Rep2>
duration<common_type_t<Rep1, Rep2>, Period>
constexpr operator/(const duration<Rep1,Period>& lhs, const Rep2& rhs)
{
    using CT = duration<common_type_t<Rep1, Rep2>, Period>;

    return CT(lhs.count() / rhs);
}

// +++ DEBT: Refactor these out to rely only on unit_base ones, if we can.  Indications look good so far,
// quick test against < worked

template <class Rep1, class Period1, class Rep2, class Period2>
constexpr bool operator>(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
        estd::chrono::duration<Rep2, Period2> >::type CT;

    return CT(lhs).count() > CT(rhs).count();
}

template <class Rep1, class Period1, class Rep2, class Period2>
constexpr bool operator<(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
        estd::chrono::duration<Rep2, Period2> >::type CT;

    return CT(lhs).count() < CT(rhs).count();
}

template <class Rep1, class Period1, class Rep2, class Period2>
constexpr bool operator<=(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    return !(lhs > rhs);
}


template <class Rep1, class Period1, class Rep2, class Period2>
constexpr bool operator>=(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    return !(lhs < rhs);
}


template <class Rep1, class Period1, class Rep2, class Period2>
constexpr bool operator==(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    using CT = common_type_t<duration<Rep1, Period1>,
        duration<Rep2, Period2> >;

    return CT(lhs).count() == CT(rhs).count();
}

template <class Rep1, class Period1, class Rep2, class Period2>
constexpr bool operator!=(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
        estd::chrono::duration<Rep2, Period2> >::type CT;

    return CT(lhs).count() != CT(rhs).count();
}

// ---


template <class C, class D1, class D2>
constexpr estd::common_type_t<D1, D2>
operator-( const time_point<C, D1>& pt_lhs, const time_point<C, D2>& pt_rhs )
{
    return pt_lhs.time_since_epoch() - pt_rhs.time_since_epoch();
}


/*
 * NOTE: Undecided if we want to do this here or inside time_point, but needed now that duration
 * init is stricter
template<class C, class D1, class Rep, class Period>
// return time_point
constexpr estd::common_type_t<D1, duration<Rep, Period>>
operator-(const time_point<C, D1>& pt_lhs,
    const duration<Rep, Period>& d_rhs )
{
    return pt_lhs.time_since_epoch() - d_rhs;
}   */


template< class Clock, class Dur1, class Dur2 >
constexpr bool operator<( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() < rhs.time_since_epoch();
}


template< class Clock, class Dur1, class Dur2 >
constexpr bool operator<=( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() <= rhs.time_since_epoch();
}


template< class Clock, class Dur1, class Dur2 >
constexpr bool operator>( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() > rhs.time_since_epoch();
}

template< class Clock, class Dur1, class Dur2 >
constexpr bool operator>=( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() >= rhs.time_since_epoch();
}


template< class Clock, class Dur1, class Dur2 >
constexpr bool operator==( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() == rhs.time_since_epoch();
}

template< class Clock, class Dur1, class Dur2 >
constexpr bool operator!=( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() != rhs.time_since_epoch();
}

}}

#include "../macro/pop.h"
