#pragma once

#include "features.h"
#include "../../ratio.h"
#include "../fwd/chrono.h"
#include "../units/traits.h"

namespace estd { namespace chrono {

namespace internal {

// DEBT: Move this tag out to 'detail'
struct seconds_tag {};

}

template <class Rep, class Period>
struct duration_traits : estd::units::v1::detail::traits<Rep, Period, internal::seconds_tag>
{
    static constexpr units::detail::options options =
        units::v1::detail::options::default_initialized;

#if FEATURE_STD_CHRONO_CORE
    using std_period = std::ratio<Period::num, Period::den>;
    using std_duration = std::chrono::duration<Rep, std_period>;
#endif

    // DEBT: May just be a formality, really burden is on common_type duration
    // specialization (not yet existing, but should since specialization doesn't
    // play nice with estd::units::detail::unit)
    // https://github.com/malachi-iot/estdlib/issues/184
    template <class Rep2, class Period2, class F = void>
    using rebind = duration_traits<Rep2, Period2>;
};

}}

