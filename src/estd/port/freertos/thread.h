#pragma once

#include "chrono.h"

namespace estd { namespace this_thread {

// untested, not yet compiled, but everything is in place so I expect this to work with little
// effort
template< class Rep, class Period >
void sleep_for( const chrono::duration<Rep, Period>& sleep_duration )
{
    // TODO: Pretty sure we can do a straight chrono::stead_clock::duration instance here
    chrono::steady_clock::rep count =
            estd::chrono::duration_cast<chrono::steady_clock::duration>(sleep_duration).count();

    vTaskDelay(count);
}

}}
