#include "chrono.h"

namespace estd { namespace chrono {

template<
        class Rep,
        class Period
>
template<
        class Rep2,
        class Period2
>
Rep duration<Rep, Period>::convert_from(const duration<Rep2, Period2>& d)
{
    typedef ratio_divide<Period2, Period> rd;

    // FIX: Overly simplistic and going to overflow in some conditions
    // put into this helper method so that (perhaps) we can specialize/overload
    // to avoid that
    //return d.count() * Period2::num() * Period::den() / (Period2::den() * Period::num());
    return d.count() * rd::num / rd::den;
}

template<
        class Rep,
        class Period
>
template<
        class Rep2,
        class Period2
>
CONSTEXPR duration<Rep, Period>::duration(const duration<Rep2, Period2>& d)
    : ticks(convert_from(d))
{
}


template <class ToDuration, class Rep, class Period>
ToDuration duration_cast(const duration<Rep, Period>& d)
{
    return ToDuration(d);
}


template< class Rep1, class Period1, class Rep2, class Period2 >
typename estd::common_type<duration<Rep1,Period1>, duration<Rep2,Period2>>::type
    CONSTEXPR operator-( const duration<Rep1,Period1>& lhs,
                         const duration<Rep2,Period2>& rhs )
{
    return typename estd::common_type<
            duration<Rep1,Period1>,
            duration<Rep2,Period2>>::type(lhs.count() - rhs.count());
}



template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator>(const duration<Rep1, Period1>& lhs,
                          const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
                             estd::chrono::duration<Rep2, Period2>>::type CT;

    return CT(lhs).count() > CT(rhs).count();
}

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator<(const duration<Rep1, Period1>& lhs,
                          const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
                             estd::chrono::duration<Rep2, Period2>>::type CT;

    return CT(lhs).count() < CT(rhs).count();
}

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator<=(const duration<Rep1, Period1>& lhs,
                          const duration<Rep2, Period2>& rhs)
{
    return !(rhs > lhs);
}


template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator>=(const duration<Rep1, Period1>& lhs,
                          const duration<Rep2, Period2>& rhs)
{
    return !(lhs > rhs);
}


template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator==(const duration<Rep1, Period1>& lhs,
                          const duration<Rep2, Period2>& rhs)
{
    typedef typename estd::common_type<estd::chrono::duration<Rep1, Period1>,
                             estd::chrono::duration<Rep2, Period2>>::type CT;

    return CT(lhs).count() == CT(rhs).count();
}


template< class C, class D1, class D2 >
CONSTEXPR typename estd::common_type<D1,D2>::type
    operator-( const time_point<C,D1>& pt_lhs,
               const time_point<C,D2>& pt_rhs )
{
    return pt_lhs.time_since_epoch() - pt_rhs.time_since_epoch();
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



}}

namespace estd {

/*
// TODO: move this to better location
template<std::intmax_t Denom1, std::intmax_t Denom2>
static CONSTEXPR std::intmax_t ratio_dividenum */

}
