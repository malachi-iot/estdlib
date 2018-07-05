#pragma once

#include "../ratio.h"

namespace estd {

namespace internal {



}

namespace chrono {

template<
        class Rep,
        class Period = estd::ratio<1>
>
class duration
{
    Rep ticks;

public:
    typedef Rep rep;
    typedef Period period;

    CONSTEXPR rep count() const { return ticks; }

    template <class Rep2, class Period2>
    CONSTEXPR duration(const duration<Rep2, Period2>& d);
};

}

}
