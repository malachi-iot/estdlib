#pragma once

#include "../../internal/chrono.h"

#ifdef FEATURE_ESTD_CHRONO

namespace estd { namespace chrono {

#if FEATURE_STD_CHRONO_CORE
template <class ToDuration, class Rep, class Period>
inline ToDuration duration_cast(const std::chrono::duration<Rep, Period>& d)
{
    typedef ratio<Period::num, Period::den> period_type;

    duration<Rep, period_type> our_d(d);

    return duration_cast<ToDuration>(our_d);
}

// Even though 'duration' has a conversion operator back to std::chrono::duration, we still
// need these overloads below

template <class Rep1, class Period1, class Rep2, class Period2>
typename std::common_type<std::chrono::duration<Rep1,Period1>, std::chrono::duration<Rep2,Period2>>::type
constexpr operator+( const std::chrono::duration<Rep1,Period1>& lhs,
    const duration<Rep2,Period2>& rhs )
{
    return lhs + std::chrono::duration<Rep2, Period2>(rhs.count());
}

template <class C, class D1, class R2, class P2>
constexpr std::chrono::time_point<C, typename std::common_type<D1, std::chrono::duration<R2,P2> >::type>
operator+(const std::chrono::time_point<C,D1>& pt, const duration<R2,P2>& d)
{
    typedef std::ratio<P2::num, P2::den> period_type;

    return pt + std::chrono::duration<R2, period_type>(d.count());
}

template <class C, class D1, class R2, class P2>
constexpr std::chrono::time_point<C, typename std::common_type<D1, std::chrono::duration<R2,P2> >::type>
operator+=(std::chrono::time_point<C,D1>& pt, const duration<R2,P2>& d)
{
    typedef std::ratio<P2::num, P2::den> period_type;

    return pt += std::chrono::duration<R2, period_type>(d.count());
}
#endif

#if FEATURE_STD_CHRONO_CLOCK
typedef std::chrono::steady_clock steady_clock;
typedef std::chrono::system_clock system_clock;
#endif

}}

#endif