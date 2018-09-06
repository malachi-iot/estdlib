#pragma once

// mainly serves as aliases to std thread

#include <thread>
#include "../chrono.h"

namespace estd {

namespace this_thread {

template< class Rep, class Period >
void sleep_for( const estd::chrono::duration<Rep, Period>& sleep_duration )
{
    typedef std::ratio<Period::num, Period::den> std_ratio;
    std::chrono::duration<Rep, std_ratio> d(sleep_duration.count());
    std::this_thread::sleep_for(d);
}

inline void yield() noexcept { std::this_thread::yield(); }

}

}
