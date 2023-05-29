#pragma once

#include "../fwd/functional.h"
#include "../macro/c++11_emul.h"

namespace estd {

struct monostate {};

ESTD_CPP_CONSTEXPR_RET bool operator==(monostate, monostate) NOEXCEPT { return true; }
ESTD_CPP_CONSTEXPR_RET bool operator!=(monostate, monostate) NOEXCEPT { return true; }

template <>
struct hash<monostate>
{
    std::size_t operator ()(monostate) const { return 0; }
};


}
