#pragma once

#include "enum.h"

namespace estd { namespace internal {

struct circular_mutex_noop
{
    static constexpr bool lock_front() { return true; }
    static constexpr bool unlock_front() { return {}; }
    static constexpr bool lock_back() { return true; }
    static constexpr bool unlock_back() { return {}; }
    static constexpr bool lock_count() { return true; }
    static constexpr bool unlock_count() { return {}; }
};

}}