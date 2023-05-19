#pragma once

#include "features.h"
#include "../../ratio.h"
#include "../chrono.h"

#include "../macro/push.h"

namespace estd { namespace chrono {

#ifdef FEATURE_ESTD_CHRONO
template<
    class Clock,
    class Duration
    > class time_point
{
    Duration m_time_since_epoch;

public:
    typedef Clock clock;
    typedef Duration duration;
    typedef typename duration::rep rep;
    typedef typename duration::period period;
    typedef estd::chrono::internal::clock_traits<clock> clock_traits;

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
        m_time_since_epoch(t.time_since_epoch())
    {}

#ifdef FEATURE_STD_CHRONO
    // Deviates from spec, normally one is not permitted to convert time points between clocks.
    // We make an exception here since one might want to convert from
    // std::chrono::time_point to estd::chrono::time_point
    // NOTE: Have to pull TClock3 shenanigan, otherwise compiler gets upset
    // claiming we're specializing clock_traits after instantiation
    // DEBT: What we really want to do is compare clock tag type itself and make
    // sure it matches - even if it's not unix_epoch_clock_tag.  Very much an edge case though
    // DEBT: Probably should do https://en.cppreference.com/w/cpp/chrono/clock_cast instead
    template <class TClock2, class TDuration2, class TClock3 = Clock,
             typename estd::enable_if<
                 estd::is_base_of<
                     estd::chrono::internal::unix_epoch_clock_tag,
                     estd::chrono::internal::clock_traits<TClock2> >::value
                     &&
                     estd::is_base_of<
                         estd::chrono::internal::unix_epoch_clock_tag,
                         //clock_traits
                         //estd::internal::clock_traits<Clock>
                         estd::chrono::internal::clock_traits<TClock3>
                         >::value
                 , bool>::type = true>
    inline time_point(const std::chrono::time_point<TClock2, TDuration2>& t) :
        m_time_since_epoch(t.time_since_epoch())
    {}
#endif

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

    // DEBT: Spec calls for this operation as outside of class
    time_point operator+(const duration& d)
    {
        time_point copied(*this);
        copied += d;
        return copied;
    }

    // DEBT: Spec calls for this operation as outside of class
    time_point operator-(const duration& d)
    {
        time_point copied(*this);
        copied -= d;
        return copied;
    }

    static inline CONSTEXPR time_point min() NOEXCEPT
    {
        return time_point(duration::min());
    }
};
#endif

}}

#include "../macro/pop.h"
