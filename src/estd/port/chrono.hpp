#include "chrono.h"

namespace estd { namespace chrono {

template<
        class Rep,
        class Period
>
template<
        class Rep2,
        class Period2
>
Rep duration<Rep, Period>::convert_from(const duration<Rep2, Period2>& d)
{
    // FIX: Overly simplistic and going to overflow in some conditions
    // put into this helper method so that (perhaps) we can specialize/overload
    // to avoid that
    return d.count() * Period2::num() * Period::den() / (Period2::den() * Period::num());
}

template<
        class Rep,
        class Period
>
template<
        class Rep2,
        class Period2
>
CONSTEXPR duration<Rep, Period>::duration(const duration<Rep2, Period2>& d)
    : ticks(convert_from(d))
{
}

}}

namespace estd {

/*
// TODO: move this to better location
template<std::intmax_t Denom1, std::intmax_t Denom2>
static CONSTEXPR std::intmax_t ratio_dividenum */

}
