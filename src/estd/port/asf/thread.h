#pragma once

#include "chrono.h"
#include <hal_delay.h> // smells ASF4 specific

namespace estd {

namespace this_thread {

// untested, not yet compiled, but everything is in place so I expect this to work with little
// effort
template< class Rep, class Period >
void sleep_for( const chrono::duration<Rep, Period>& sleep_duration )
{
    // TODO: Pretty sure we can do a straight chrono::stead_clock::duration instance here
    chrono::milliseconds count =
            estd::chrono::duration_cast<chrono::milliseconds>(sleep_duration).count();

    delay_ms(count);
}

// noop since there's no rtos
inline void yield() {}

}

}
