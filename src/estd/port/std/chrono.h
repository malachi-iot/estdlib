#pragma once

#include "../../internal/chrono.h"
#include "../../internal/fwd/chrono.h"

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
// need these overloads below.  Caveats:
// 1.  AI indicates this is undefined behavior specializing these operators
// 2.  In std::chrono, no common_type occurs for +=/-=, but we do it

namespace detail {

template <class Rep1, class Period1, class Traits>
std::common_type_t<std::chrono::duration<Rep1,Period1>, typename Traits::std_duration>
constexpr operator+(const std::chrono::duration<Rep1,Period1>& lhs,
    const duration<Traits>& rhs )
{
    return lhs + typename Traits::std_duration(rhs.count());
}

template <class C, class D1, class Traits>
constexpr std::chrono::time_point<
    C,
    std::common_type_t<D1, typename Traits::std_duration>>
operator+(const std::chrono::time_point<C,D1>& pt, const duration<Traits>& d)
{
    return pt + typename Traits::std_duration(d.count());
}

template <class C, class D1, class Traits>
constexpr std::chrono::time_point<C, std::common_type_t<D1, typename Traits::std_duration>>
operator-(const std::chrono::time_point<C,D1>& pt, const duration<Traits>& d)
{
    return pt - typename Traits::std_duration(d.count());
}

template <class C, class D1, class Traits>
constexpr std::chrono::time_point<
    C,
    std::common_type<D1, std::chrono::duration<typename Traits::rep, typename Traits::period>>>
operator+=(std::chrono::time_point<C,D1>& pt, const duration<Traits>& d)
{
    using p2 = typename Traits::period;
    typedef std::ratio<p2::num, p2::den> period_type;

    return pt += std::chrono::duration<typename Traits::rep, period_type>(d.count());
}

template <class C, class D1, class Traits>
constexpr std::chrono::time_point<
    C,
    std::common_type_t<D1, std::chrono::duration<typename Traits::rep, typename Traits::period>>>
operator-=(std::chrono::time_point<C, D1>& pt, const duration<Traits>& d)
{
    using p2 = typename Traits::period;
    typedef std::ratio<p2::num, p2::den> period_type;

    return pt -= std::chrono::duration<typename Traits::rep, period_type>(d.count());
}

}
#endif

#if FEATURE_STD_CHRONO_CLOCK
typedef std::chrono::steady_clock steady_clock;
typedef std::chrono::system_clock system_clock;
#endif

}}

#endif
