#pragma once

// While developing, leave this on
#define FEATURE_ESTD_CHRONO

#include "../ratio.h"

namespace estd {

namespace internal {

// embedded-oriented version has lower precision.  Deviates from standard
// Revisit this on a per-platform and
// option-selectable level
typedef uint32_t nano_rep;
typedef uint32_t micro_rep;
typedef uint32_t miilli_rep;
typedef uint16_t seconds_rep;
typedef uint16_t minutes_rep;
typedef uint16_t hours_rep;
typedef uint8_t days_rep;
typedef uint8_t years_rep;


}

#ifdef FEATURE_ESTD_CHRONO

namespace chrono {

template<
        class Rep,
        class Period = estd::ratio<1>
>
class duration
{
    // confusingly, 'ticks' actually represents # of periods, not specifically
    // system ticks
    Rep ticks;

protected:
    template <class Rep2, class Period2>
    static Rep convert_from(const duration<Rep2, Period2>& d);

public:
    typedef Rep rep;
    typedef Period period;

    CONSTEXPR rep count() const { return ticks; }

    template <class Rep2>
    CONSTEXPR explicit duration(const Rep2& r) : ticks(r)
    {

    }

    template <class Rep2, class Period2>
    CONSTEXPR duration(const duration<Rep2, Period2>& d);

};

template <class ToDuration, class Rep, class Period>
ToDuration duration_cast(const duration<Rep, Period>& d);


template< class Rep1, class Period1, class Rep2, class Period2 >
typename estd::common_type<duration<Rep1,Period1>, duration<Rep2,Period2>>::type
    CONSTEXPR operator-( const duration<Rep1,Period1>& lhs,
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


#endif // FEATURE_ESTD_CHRONO

}

namespace internal {

// Use this to turn on incomplete estd::chrono support namespace
// there's a lot that goes into a healthy, functioning chrono namespace
// so we default to using std::chrono
#ifdef FEATURE_ESTD_CHRONO
namespace estd_ratio = estd;
namespace estd_chrono = estd::chrono;
#else
namespace estd_ratio = std;
namespace estd_chrono = std::chrono;
#endif

}

namespace chrono {

// These lower-precision ones are available even during non-FEATURE_ESTD_CHRONO
typedef internal::estd_chrono::duration<internal::nano_rep, nano> nanoseconds;
typedef internal::estd_chrono::duration<internal::micro_rep, micro> microseconds;
typedef internal::estd_chrono::duration<internal::miilli_rep, milli> milliseconds;
typedef internal::estd_chrono::duration<internal::seconds_rep> seconds;
typedef internal::estd_chrono::duration<internal::minutes_rep, ratio<60>> minutes;
typedef internal::estd_chrono::duration<internal::hours_rep, ratio<3600>> hours;
typedef internal::estd_chrono::duration<internal::days_rep, ratio<3600 * 24>> days;

#ifdef FEATURE_ESTD_CHRONO

template<
    class Clock,
    class Duration = typename Clock::duration
> class time_point
{
    Duration m_time_since_epoch;

public:
    typedef Clock clock;
    typedef Duration duration;
    typedef typename duration::rep rep;
    typedef typename duration::period period;

    time_point(const Duration& duration) : m_time_since_epoch(duration) {}

    Duration time_since_epoch() const { return m_time_since_epoch; }
};


template< class C, class D1, class D2 >
constexpr typename estd::common_type<D1,D2>::type
    operator-( const time_point<C,D1>& pt_lhs,
               const time_point<C,D2>& pt_rhs );

template< class Clock, class Dur1, class Dur2 >
constexpr bool operator>( const time_point<Clock,Dur1>& lhs,
                          const time_point<Clock,Dur2>& rhs );

template< class Clock, class Dur1, class Dur2 >
constexpr bool operator>=( const time_point<Clock,Dur1>& lhs,
                          const time_point<Clock,Dur2>& rhs );
}

}

#include "chrono.hpp"

#endif // FEATURE_ESTD_CHRONO
