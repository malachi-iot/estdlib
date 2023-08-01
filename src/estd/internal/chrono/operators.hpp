#pragma once

#include "../fwd/chrono.h"
#include "duration.hpp"
#include "ratio.h"
#include "../macro/push.h"

namespace estd { namespace chrono {

// NOTE: Not well tested for operations across different duration types
template< class Rep1, class Period1, class Rep2, class Period2 >
typename estd::common_type<duration<Rep1,Period1>, duration<Rep2,Period2> >::type
CONSTEXPR operator-( const duration<Rep1,Period1>& lhs,
    const duration<Rep2,Period2>& rhs )
{
    typedef typename common_type<duration<Rep1, Period1>,
        duration<Rep2, Period2> >::type CT;

    return CT(CT(lhs).count() - CT(rhs).count());
}



template< class Rep1, class Period1, class Rep2, class Period2 >
typename estd::common_type<duration<Rep1,Period1>, duration<Rep2,Period2> >::type
CONSTEXPR operator+( const duration<Rep1,Period1>& lhs,
    const duration<Rep2,Period2>& rhs )
{
    typedef typename common_type<duration<Rep1, Period1>,
        duration<Rep2, Period2> >::type CT;

    return CT(CT(lhs).count() + CT(rhs).count());
}


template< class Rep1, class Period, class Rep2>
duration<typename estd::common_type<Rep1,Rep2>::type, Period>
CONSTEXPR operator*( const duration<Rep1,Period>& lhs,
    const Rep2& rhs )
{
    typedef duration<
        typename estd::common_type<Rep1,Rep2>::type, Period>
        CT;

    return CT(lhs.count() * rhs);
}



template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator>(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
        estd::chrono::duration<Rep2, Period2> >::type CT;

    return CT(lhs).count() > CT(rhs).count();
}

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator<(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
        estd::chrono::duration<Rep2, Period2> >::type CT;

    return CT(lhs).count() < CT(rhs).count();
}

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator<=(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    return !(lhs > rhs);
}


template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator>=(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    return !(lhs < rhs);
}


template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator==(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
        estd::chrono::duration<Rep2, Period2> >::type CT;

    return CT(lhs).count() == CT(rhs).count();
}

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator!=(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
        estd::chrono::duration<Rep2, Period2> >::type CT;

    return CT(lhs).count() != CT(rhs).count();
}


template< class C, class D1, class D2 >
CONSTEXPR typename estd::common_type<D1,D2>::type
operator-( const time_point<C,D1>& pt_lhs,
    const time_point<C,D2>& pt_rhs )
{
    return pt_lhs.time_since_epoch() - pt_rhs.time_since_epoch();
}


template< class Clock, class Dur1, class Dur2 >
CONSTEXPR bool operator<( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() < rhs.time_since_epoch();
}


template< class Clock, class Dur1, class Dur2 >
CONSTEXPR bool operator>( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() > rhs.time_since_epoch();
}

template< class Clock, class Dur1, class Dur2 >
CONSTEXPR bool operator>=( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() >= rhs.time_since_epoch();
}


template< class Clock, class Dur1, class Dur2 >
CONSTEXPR bool operator==( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() == rhs.time_since_epoch();
}

template< class Clock, class Dur1, class Dur2 >
CONSTEXPR bool operator!=( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs )
{
    return lhs.time_since_epoch() != rhs.time_since_epoch();
}

}}

#include "../macro/pop.h"
