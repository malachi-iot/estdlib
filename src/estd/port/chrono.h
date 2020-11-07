#pragma once

// While developing, leave this on
#define FEATURE_ESTD_CHRONO

// Fully activating more advanced common_type/promoted_type fused
// duration conversions
#define FEATURE_ESTD_CHRONO_EXP

#include "../internal/common_type.h"
#include "../ratio.h"
#include "../limits.h"

#if defined(ESTD_POSIX) && !defined(FEATURE_ESTD_NATIVE_CHRONO) && __cplusplus >= 201103L
// DEBT: Doing this define here is the wrong spot - should be earlier in port/platform chain
#define FEATURE_CPP_CHRONO
#include <chrono>
#endif

#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
#pragma push_macro("abs")
#pragma push_macro("max")
#pragma push_macro("min")
#undef abs
#undef max
#undef min
#endif


namespace estd {

namespace internal {

// embedded-oriented version has lower precision.  Deviates from standard
// Revisit this on a per-platform and
// option-selectable level
#ifdef FEATURE_ESTD_CHRONO_LOWPRECISION
typedef int32_t nano_rep;
typedef int32_t micro_rep;
typedef int32_t milli_rep;
typedef int16_t seconds_rep;
typedef int16_t minutes_rep;
typedef int16_t hours_rep;
typedef int16_t days_rep;
typedef int8_t weeks_rep;
typedef int8_t years_rep;
#else
typedef int64_t nano_rep;
typedef int64_t micro_rep;
typedef int32_t milli_rep;
typedef int32_t seconds_rep;
typedef int32_t minutes_rep;
typedef int32_t hours_rep;
typedef int32_t days_rep;
typedef int16_t weeks_rep;
typedef int16_t years_rep;
#endif


}

#ifdef FEATURE_ESTD_CHRONO

namespace chrono {

template <class Rep>
struct duration_values
{
    static CONSTEXPR Rep zero() { return Rep(0); }
    static CONSTEXPR Rep min() { return estd::numeric_limits<Rep>::min(); }
    static CONSTEXPR Rep max() { return estd::numeric_limits<Rep>::max(); }
};

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
    static CONSTEXPR Rep convert_from(const duration<Rep2, Period2>& d);

#ifdef FEATURE_CPP_CHRONO
    template <class Rep2, class Period2>
    static CONSTEXPR Rep convert_from(const std::chrono::duration<Rep2, Period2>& d);
#endif

public:
    typedef Rep rep;
    typedef Period period;

    CONSTEXPR rep count() const { return ticks; }

#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    duration()
#ifdef FEATURE_CPP_DEFAULT_FUNCDEF
        = default;
#else
    {}
#endif

    template <class Rep2>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    explicit duration(const Rep2& r) : ticks(r)
    {

    }

    template <class Rep2, class Period2>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    duration(const duration<Rep2, Period2>& d);

#ifdef FEATURE_CPP_CHRONO
    template <class Rep2, class Period2>
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    duration(const std::chrono::duration<Rep2, Period2>& d) :
        ticks(convert_from(d))
    {}

    typedef std::ratio<Period::num, Period::den> std_period_type;

    operator std::chrono::duration<Rep, std_period_type>() const
    {
        std::chrono::duration<Rep, std_period_type> converted(count());

        return converted;
    }
#endif

    duration& operator+=(const duration& d)
    {
        ticks += d.ticks;
        return *this;
    }


    duration& operator-=(const duration& d)
    {
        ticks -= d.ticks;
        return *this;
    }

    duration& operator*=(const rep& r)
    {
        ticks *= r;
        return *this;
    }

    CONSTEXPR duration operator-() const
    {
#ifdef FEATURE_CPP_STATIC_ASSERT
        static_assert (numeric_limits<rep>::is_signed, "operator -() requires a signed Rep type");
#endif

        return duration(-ticks);
    }

    CONSTEXPR duration operator+() const { return *this; }

    static CONSTEXPR duration min() { return duration(duration_values<rep>::min()); }
    static CONSTEXPR duration max() { return duration(duration_values<rep>::max()); }
    static CONSTEXPR duration zero() { return duration(duration_values<rep>::zero()); }
};

template <class ToDuration, class Rep, class Period>
ToDuration duration_cast(const duration<Rep, Period>& d);

#ifdef FEATURE_CPP_CHRONO
template <class ToDuration, class Rep, class Period>
inline ToDuration duration_cast(const std::chrono::duration<Rep, Period>& d)
{
    typedef ratio<Period::num, Period::den> period_type;

    duration<Rep, period_type> our_d(d);

    return duration_cast<ToDuration>(our_d);
}
#endif

template< class Rep1, class Period1, class Rep2, class Period2 >
typename estd::common_type<duration<Rep1,Period1>, duration<Rep2,Period2> >::type
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

template <class Rep1, class Period1, class Rep2, class Period2>
CONSTEXPR bool operator==(const duration<Rep1, Period1>& lhs,
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
typedef internal::estd_chrono::duration<internal::milli_rep, milli> milliseconds;
typedef internal::estd_chrono::duration<internal::seconds_rep> seconds;
typedef internal::estd_chrono::duration<internal::minutes_rep, ratio<60>> minutes;
typedef internal::estd_chrono::duration<internal::hours_rep, ratio<3600>> hours;
// NOTE: AVR compiler requires this long typecast.  Doesn't hurt anything (though it's
// a bit ugly)
typedef internal::estd_chrono::duration<internal::days_rep, ratio<(long)3600 * 24>> days;

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

    // NOTE: *may* deviate from spec.  Leaves m_time_since_epoch undefined
    // spec, to my ears, is unclear:
    //   "Default constructor, creates a time_point with a value of Clock's epoch."
    // this implies a 'now' operation, but I am not sure.  Since we are targetting embedded,
    // less is more, so swing vote goes to doing nothing here.  This turns out to be helpful,
    // now one can leave a time_point as a global variable without worring about startup-time
    // init issues
    // NOTE: considering above, I am interpreting 'value of Clock's epoch' to generally
    // mean 0 i.e. the starting point from which the clock begins - unix epoch is around 1970,
    // and the seconds count up from there so 0 = 1970.  The inspecific nature of things
    // implies there's a way to reach into Clock itself and grab this epoch
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    time_point() : m_time_since_epoch(0) {}

#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#endif
    explicit time_point(const Duration& duration) : m_time_since_epoch(duration) {}

    // NOTE: Compiles, but not tested
    template <class TDuration2>
    time_point(const time_point<Clock, TDuration2>& t) :
        m_time_since_epoch(t.m_time_since_epoch)
    {}

    Duration time_since_epoch() const { return m_time_since_epoch; }


    //constexpr
    time_point& operator+=( const duration& d )
    {
        m_time_since_epoch += d;
        return *this;
    }

    time_point& operator-=( const duration& d )
    {
        m_time_since_epoch -= d;
        return *this;
    }
};


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
}

namespace literals {

namespace chrono_literals {

// spammy warnings make this too obnoxious to enable right now
#ifdef UNUSED

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"

#ifdef FEATURE_CPP_USER_LITERAL
constexpr chrono::seconds operator "" s(unsigned long long s)
{
    return chrono::seconds(s);
}

constexpr chrono::seconds operator "" ms(unsigned long long ms)
{
    return chrono::milliseconds(ms);
}

#pragma GCC diagnostic pop

#endif

#endif

}

}


}

#include "chrono.hpp"

#if defined(__GNUC__) || defined(_MSC_VER)
#pragma pop_macro("min")
#pragma pop_macro("max")
#pragma pop_macro("abs")
#endif

#endif // FEATURE_ESTD_CHRONO
