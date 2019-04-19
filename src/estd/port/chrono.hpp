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
CONSTEXPR Rep duration<Rep, Period>::convert_from(const duration<Rep2, Period2>& d)
{
    typedef ratio_divide<Period2, Period> rd;

    // So this isn't the answer but it's close
    // 1) a warning would be much preferred
    // 2) duration_cast should suppress this, but doesn't
#ifdef _FEATURE_CPP_STATIC_ASSERT
    static_assert (estd::numeric_limits<Rep>::digits >= estd::numeric_limits<Rep2>::digits,
                   "Precision loss");
#endif

    // FIX: Overly simplistic and going to overflow in some conditions
    // put into this helper method so that (perhaps) we can specialize/overload
    // to avoid that
    //return d.count() * Period2::num() * Period::den() / (Period2::den() * Period::num());
    // A little surprising that this statement doesn't issue compiler warnings about precision loss
    // even when Rep is much smaller of an integer type than Rep2
    return d.count() * rd::num / rd::den;
}

#ifdef ESTD_POSIX
template<
        class Rep,
        class Period
>
template<
        class Rep2,
        class Period2
>
CONSTEXPR Rep duration<Rep, Period>::convert_from(const std::chrono::duration<Rep2, Period2>& d)
{
    typedef std::ratio_divide<Period2, Period> rd;

    // FIX: Overly simplistic and going to overflow in some conditions
    // put into this helper method so that (perhaps) we can specialize/overload
    // to avoid that
    //return d.count() * Period2::num() * Period::den() / (Period2::den() * Period::num());
    return d.count() * rd::num / rd::den;
}
#endif

template<
        class Rep,
        class Period
>
template<
        class Rep2,
        class Period2
>
#ifdef FEATURE_CPP_CONSTEXPR
constexpr
#endif
duration<Rep, Period>::duration(const duration<Rep2, Period2>& d)
    : ticks(convert_from(d))
{
}


template <class ToDuration, class Rep, class Period>
ToDuration duration_cast(const duration<Rep, Period>& d)
{
    return ToDuration(d);
}


// NOTE: Not well tested for operations across different duration types
template< class Rep1, class Period1, class Rep2, class Period2 >
typename estd::common_type<duration<Rep1,Period1>, duration<Rep2,Period2>>::type
    CONSTEXPR operator-( const duration<Rep1,Period1>& lhs,
                         const duration<Rep2,Period2>& rhs )
{
    typedef typename common_type<duration<Rep1, Period1>,
            duration<Rep2, Period2>>::type CT;

    return CT(CT(lhs).count() - CT(rhs).count());
}



template< class Rep1, class Period1, class Rep2, class Period2 >
typename estd::common_type<duration<Rep1,Period1>, duration<Rep2,Period2>>::type
CONSTEXPR operator+( const duration<Rep1,Period1>& lhs,
                     const duration<Rep2,Period2>& rhs )
{
    typedef typename common_type<duration<Rep1, Period1>,
            duration<Rep2, Period2>>::type CT;

    return CT(CT(lhs).count() + CT(rhs).count());
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


// lifted from https://en.cppreference.com/w/cpp/chrono/duration/abs
// FIX: Not going to work until we implement min() and zero()
template <class Rep, class Period, class = estd::enable_if_t<
   duration<Rep, Period>::min() < duration<Rep, Period>::zero()>>
CONSTEXPR duration<Rep, Period> abs(duration<Rep, Period> d)
{
    return d >= d.zero() ? d : -d;
}

}}

namespace estd {

#ifdef FEATURE_ESTD_CHRONO_EXP

/*
// TODO: move this to better location
template<std::intmax_t Denom1, std::intmax_t Denom2>
static CONSTEXPR std::intmax_t ratio_dividenum */

/*
// for duration to be a common type, we need the same denominator.  This means
// one of the ratio's numerators must increase, thus reducing the precision on the
// other ratio.
template <typename Dur1Int, typename Dur2Int, int Num1, int Num2, int Denom1, int Denom2>
struct common_type<
        chrono::duration<Dur1Int, ratio<Num1, Denom1> >,
        chrono::duration<Dur2Int, ratio<Num2, Denom2> > >
{
private:
    static CONSTEXPR int gcd_den = internal::gcd<Denom2, Denom1>::value;
    static CONSTEXPR int _Num1 = Num1 * (Denom2 / gcd_den);
    static CONSTEXPR int _Num2 = Num2 * (Denom1 / gcd_den);
    static CONSTEXPR bool num1gtnum2 = _Num1 > _Num2;
    static CONSTEXPR int NewNum = num1gtnum2 ? _Num1 : _Num2;
    static CONSTEXPR int NewDenom = (Denom1 * Denom2) / gcd_den;
    //static CONSTEXPR int gcd = internal::gcd<NewNum, NewDenom>::value;
    static CONSTEXPR int gcd = 1;

public:
    // dummy type, for now
    typedef chrono::duration<unsigned, estd::ratio<NewNum / gcd, NewDenom / gcd> > type;
}; */


// for duration to be a common type, we need the same denominator.  This means
// one of the ratio's numerators must increase, thus reducing the precision on the
// other ratio.
// custom "promoted_type" is utilized to keep duration's 'Rep' from bloating
template <typename Dur1Int, typename Dur2Int,
        std::intmax_t Num1, std::intmax_t Num2,
        std::intmax_t Denom1, std::intmax_t Denom2>
struct common_type<
        chrono::duration<Dur1Int, ratio<Num1, Denom1> >,
        chrono::duration<Dur2Int, ratio<Num2, Denom2> > >
{
private:
    // gracefully promote (or not) types used.  non-specialized common_type is very
    // aggressive about promoting and almost always adds bits - otherwise we'd use it
    typedef typename promoted_type<Dur1Int, Dur2Int>::type common_int_type;

    // greatest common divisor of denominator
    static CONSTEXPR std::intmax_t gcd_den = internal::gcd<Denom2, Denom1>::value;
    //static CONSTEXPR int gcd_num = internal::gcd<Num1, Num2>::value;
    static CONSTEXPR std::intmax_t NewDenom = Denom1 * Denom2;
    static CONSTEXPR std::intmax_t NewNum = Num1 * Num2 * gcd_den;
    static CONSTEXPR std::intmax_t gcd = internal::gcd<NewDenom, NewNum>::value;

public:
    typedef chrono::duration<common_int_type, estd::ratio<NewNum / gcd, NewDenom / gcd> > type;
};

#endif

}
