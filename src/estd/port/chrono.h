#pragma once

#include "../ratio.h"

namespace estd {

template<
    class Rep,
    class Period = estd::ratio<1>
> class duration
{
    Rep ticks;

public:
    typedef Rep rep;
    typedef Period period;

    CONSTEXPR rep count() const { return ticks; }
};

}
