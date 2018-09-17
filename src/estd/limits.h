#pragma once

#include "internal/platform.h"
#include "cstdint.h"

namespace estd {

namespace numeric_limits {

template <class T>
CONSTEXPR T max();

template<>
inline CONSTEXPR uint16_t max<uint16_t>()
{ return 65535; }

template<>
inline CONSTEXPR uint8_t max<uint8_t>()
{ return 255; }

}

}
