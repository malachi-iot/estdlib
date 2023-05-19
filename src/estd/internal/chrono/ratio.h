#pragma once

#include "features.h"
#include "../fwd/chrono.h"
#include "../../ratio.h"

namespace estd {
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

// Old bugged code
#if LEGACY
    // greatest common divisor of denominator
    static CONSTEXPR std::intmax_t gcd_den = internal::gcd<Denom2, Denom1>::value;
    //static CONSTEXPR int gcd_num = internal::gcd<Num1, Num2>::value;
    static CONSTEXPR std::intmax_t NewDenom = Denom1 * Denom2;
    static CONSTEXPR std::intmax_t NewNum = Num1 * Num2 * gcd_den;
    static CONSTEXPR std::intmax_t gcd = internal::gcd<NewDenom, NewNum>::value;
#endif

    static CONSTEXPR std::intmax_t gcd_num = internal::gcd<Num1, Num2>::value;
    static CONSTEXPR std::intmax_t lcm_den = internal::lcm<Denom1, Denom2>::value;

public:
    typedef estd::ratio<gcd_num, lcm_den> ratio_type;

#if LEGACY
    typedef chrono::duration<common_int_type, estd::ratio<NewNum / gcd, NewDenom / gcd> > type;
#endif
    typedef chrono::duration<common_int_type, ratio_type > type;
};

// Additional specializations to interact with std::ratio, if present
#ifdef FEATURE_STD_RATIO
template <typename Dur1Int, typename Dur2Int,
         std::intmax_t Num1, std::intmax_t Num2,
         std::intmax_t Denom1, std::intmax_t Denom2>
struct common_type<
    chrono::duration<Dur1Int, std::ratio<Num1, Denom1> >,
    chrono::duration<Dur2Int, ratio<Num2, Denom2> > > :
    common_type<chrono::duration<Dur1Int, ratio<Num1, Denom1> >,
                chrono::duration<Dur2Int, ratio<Num2, Denom2> > >
{};

template <typename Dur1Int, typename Dur2Int,
         std::intmax_t Num1, std::intmax_t Num2,
         std::intmax_t Denom1, std::intmax_t Denom2>
struct common_type<
    chrono::duration<Dur1Int, ratio<Num1, Denom1> >,
    chrono::duration<Dur2Int, std::ratio<Num2, Denom2> > > :
    common_type<chrono::duration<Dur1Int, ratio<Num1, Denom1> >,
             chrono::duration<Dur2Int, ratio<Num2, Denom2> > >
{};
#endif

}
