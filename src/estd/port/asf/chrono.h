#pragma once

#include "../chrono.h"

namespace estd {

namespace chrono {

// dummy definition for now for native clock
struct asf_clock
{
    typedef ::estd::internal::milli_rep rep;
    typedef ::estd::milli period;

    typedef internal::estd_chrono::duration<rep, period> duration;
    typedef internal::estd_chrono::time_point<asf_clock> time_point;

    // implement now() maybe based on
    // http://asf.atmel.com/docs/latest/samg/html/time__tick__sam_8c.html
    // though it looks like asf4 doesn't generate a system timer at all except
    // to wire it to FreeRTOS
};

}

}
