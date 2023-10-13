#pragma once

#include "../../internal/platform.h"

#include <thread>
#include "../chrono.h"

namespace estd {

typedef std::thread thread;

namespace this_thread {

template <class Rep, class Period>
void sleep_for(const estd::chrono::duration<Rep, Period>& sleep_duration)
{
    typedef std::ratio<Period::num, Period::den> std_ratio;
    std::chrono::duration<Rep, std_ratio> d(sleep_duration.count());
    std::this_thread::sleep_for(d);
}

// DEBT: Not sure how we'd alias a sleep_until, time_points don't move between
// clocks so comfortably.  Could experiment with just slapping incoming
// estd::chrono::time_point 'Clock' into a std::chrono::time_point

inline void yield() NOEXCEPT { std::this_thread::yield(); }

inline thread::id get_id() NOEXCEPT
{
    return std::this_thread::get_id();
}

}

}
