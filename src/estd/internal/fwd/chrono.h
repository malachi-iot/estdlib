#pragma once

#include "common_type.h"
#include "ratio.h"

#if __cpp_lib_concepts
#include <concepts>
#endif

namespace estd { namespace chrono {

template<
    class Rep,
    class Period = estd::ratio<1>
>
class duration;

template<
    class Clock,
    class Duration = typename Clock::duration
> class time_point;

#if __cpp_concepts
namespace internal { namespace concepts { inline namespace v1 {

// these concepts may be useful in cases where std vs estd time_point
// is passed around

// DEBT: This guy belongs elsewhere
template <class T>
concept Ratio = requires(T t)
{
    T::num;
    T::den;
};

// TODO: Handle return types in below concepts - not doing so yet because there
// was an odd edge case in which <concepts> was not available (not just AVR) even
// though c++20 was.

template <class T>
concept Duration =
    Ratio<typename T::period> &&
    requires(T t)
{
    typename T::rep;

    { t.count() };
};


// Corresponds to https://en.cppreference.com/w/cpp/named_req/Clock
// https://en.cppreference.com/w/cpp/chrono/is_clock
template <class T>
concept Clock =
    Duration<typename T::duration> &&
    requires(T t)
{
    typename T::time_point;

    // DEBT: Over permissive, clock really needs a static not instance method
    t.now();
};

template <class T>
concept TimePoint =
    Duration<typename T::duration> &&
    Clock<typename T::clock> &&
    requires(T t)
{
    { t.time_since_epoch() };
};

}}}
#endif


// DEBT: Would do estd::chrono::internal, but that collides with our estd_chrono trick in port area
namespace internal {

// To be applied to traits specifically
struct unix_epoch_clock_tag {};

// Since C++20 epoch is midnight 1/1/1970 GMT, and is loosely considered standardized on that
// prior to C++20.  I haven't found any API in the spec which actually reflects that though, so
// we make our own.
// DEBT: Just found a very similar one, https://en.cppreference.com/w/cpp/chrono/clock_time_conversion
template <class TClock>
struct clock_traits;


// embedded-oriented version has lower precision.  Deviates from standard
// DEBT: Revisit this on a per-platform and option-selectable level
#ifdef FEATURE_ESTD_CHRONO_LOWPRECISION
typedef int32_t nano_rep;
typedef int32_t micro_rep;
typedef int32_t milli_rep;
typedef int16_t seconds_rep;
typedef int16_t minutes_rep;
typedef int16_t hours_rep;
typedef int16_t days_rep;
typedef int8_t weeks_rep;
typedef int8_t months_rep;
typedef int8_t years_rep;
#else
typedef int64_t nano_rep;
typedef int64_t micro_rep;
typedef int64_t milli_rep;
typedef int64_t seconds_rep;
typedef int32_t minutes_rep;
typedef int32_t hours_rep;
typedef int32_t days_rep;
typedef int32_t weeks_rep;
typedef int32_t months_rep;
typedef int16_t years_rep;      ///< Deviates from spec which calls for 17 bit minimum
#endif


}

template <class ToDuration, class Rep, class Period>
constexpr ToDuration duration_cast(const duration<Rep, Period>& d);

template< class Rep1, class Period1, class Rep2, class Period2 >
typename estd::common_type<duration<Rep1,Period1>, duration<Rep2,Period2> >::type
CONSTEXPR operator-( const duration<Rep1,Period1>& lhs,
    const duration<Rep2,Period2>& rhs );

template< class Rep1, class Period1, class Rep2, class Period2 >
typename estd::common_type<duration<Rep1,Period1>, duration<Rep2,Period2> >::type
CONSTEXPR operator+( const duration<Rep1,Period1>& lhs,
    const duration<Rep2,Period2>& rhs );

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator>(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs);

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator<(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs);

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator>=(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs);

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator<=(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs);

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator==(const duration<Rep1, Period1>& lhs,
    const duration<Rep2, Period2>& rhs);

template< class C, class D1, class D2 >
CONSTEXPR typename estd::common_type<D1,D2>::type
operator-( const time_point<C,D1>& pt_lhs,
    const time_point<C,D2>& pt_rhs );


template< class Clock, class Dur1, class Dur2 >
CONSTEXPR bool operator>( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs );

template< class Clock, class Dur1, class Dur2 >
CONSTEXPR bool operator>=( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs );

template< class Clock, class Dur1, class Dur2 >
CONSTEXPR bool operator==( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs );

template< class Clock, class Dur1, class Dur2 >
CONSTEXPR bool operator!=( const time_point<Clock,Dur1>& lhs,
    const time_point<Clock,Dur2>& rhs );


}}
