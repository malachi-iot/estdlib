#pragma once

#include "fwd.h"

#include "../type_traits.h"

namespace estd { namespace internal {

// DEBT: CREATE mode is experimental and is_default_constructible falls apart for forwarding constructors,
// resulting in compile time errors in that case when base class really isn't default constructible.
// Therefore we play some games here.
template <class T, class IsDefaultConstructible>
struct rtto_traits
{
    using value_type = T;

    // DEBT: Switch these to estd variety for AVR compability
    using is_copy_constructible = estd::is_copy_constructible<value_type>;
    using is_move_constructible = estd::is_move_constructible<value_type>;
    using is_trivially_constructible = estd::is_trivially_constructible<value_type>;
    using is_trivially_moveable = estd::is_trivially_move_constructible<value_type>;
    //using is_constructible = std::is_default_constructible<value_type>;
    using is_constructible = IsDefaultConstructible;
#if __cpp_lib_is_swappable
    using is_swappable = std::is_swappable<value_type>;
#else
    using is_swappable = std::false_type;
#endif

    // See metadata for commentary
    static constexpr unsigned complexity = 0;
};

}}
