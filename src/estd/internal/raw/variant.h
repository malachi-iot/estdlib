#pragma once

#include "../fwd/functional.h"
#include "cstddef.h"

namespace estd {

struct monostate {};

ESTD_CPP_CONSTEXPR_RET bool operator==(monostate, monostate) NOEXCEPT { return true; }
ESTD_CPP_CONSTEXPR_RET bool operator!=(monostate, monostate) NOEXCEPT { return true; }

template <>
struct hash<monostate>
{
    ESTD_CPP_CONSTEXPR_RET size_t operator ()(monostate) const { return 0; }
};

namespace internal {

// DEBT: Supposed to be an inline variable, but we want c++11 compat, so we deviate from spec
ESTD_CPP_CONSTEXPR_RET size_t variant_npos() { return (size_t)-1; }

}

#if __cpp_constexpr >= 201603L
constexpr size_t variant_npos = internal::variant_npos();
#endif


}
