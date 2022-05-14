#pragma once

#include "internal/platform.h"
#include "internal/functional.h"

namespace estd {

struct monostate {};

inline CONSTEXPR bool operator==(monostate, monostate) NOEXCEPT { return true; }
inline CONSTEXPR bool operator!=(monostate, monostate) NOEXCEPT { return true; }

template <>
struct hash<monostate>
{
    std::size_t operator ()(monostate) const { return 0; }
};

}