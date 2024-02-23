#pragma once

#include "../fwd/functional.h"

// Marked internal temporarily as we implement things

namespace estd {

// NOTE: Somewhat experimental, just whipping something up

template <>
struct hash<int>
{
    using precision = unsigned;

    constexpr precision operator()(const int& v) const
    {
        return (precision) v;
    }
};

}
