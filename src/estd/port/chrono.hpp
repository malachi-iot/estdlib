#pragma once

#include "../internal/fwd/chrono.h"
#include "../internal/chrono/duration.hpp"
#include "../internal/chrono/ratio.h"
#include "../internal/macro/push.h"

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


#if __cpp_constexpr
// lifted from https://en.cppreference.com/w/cpp/chrono/duration/abs
template <class Rep, class Period, class = estd::enable_if_t<
    estd::numeric_limits<Rep>::is_signed &&
    duration<Rep, Period>::min() < duration<Rep, Period>::zero()> >
constexpr duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return d >= d.zero() ? d : -d;
}
#else
// UNTESTED
// DEBT: I think even c++03 can check for min()/zero() at compile time, so if
// so we can merge this and above version
template <class Rep, class Period>
inline typename estd::enable_if<
    estd::numeric_limits<Rep>::is_signed, duration<Rep, Period> >::type
    abs(duration<Rep, Period> d)
{
    return d >= d.zero() ? d : -d;
}
#endif

namespace internal {

// DEBT: We'd like this to be c++03 friendly
#if __cplusplus >= 201103L

template <class Rep, class Period, typename estd::enable_if<
    estd::numeric_limits<Rep>::is_signed, bool>::type = true>
CONSTEXPR duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return chrono::abs(d);
}


template <class Rep, class Period, typename estd::enable_if<
    !estd::numeric_limits<Rep>::is_signed, bool>::type = true>
CONSTEXPR duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return d;
}

#endif


}

}}


#include "../internal/macro/pop.h"
