#pragma once

#include "../units/common_type.h"
#include "traits.h"

namespace estd {

template <class Traits1, class Traits2>
struct common_type<
    chrono::detail::duration<Traits1>,
    chrono::detail::duration<Traits2>> :
    private common_type<
        units::detail::unit<Traits1>,
        units::detail::unit<Traits2>>
{
    using base_type = common_type<
        units::detail::unit<Traits1>,
        units::detail::unit<Traits2>>;

    // DEBT: Hardcoding to traits1 for the rebind is imperfect
    using traits = typename Traits1::template rebind<
        typename base_type::rep,
        typename base_type::period>;

    using type = chrono::detail::duration<traits>;
};

}
