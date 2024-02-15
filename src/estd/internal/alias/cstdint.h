#pragma once

#include <stdint.h>         // NOLINT

#if FEATURE_ESTD_STD_ALIAS

namespace std {

using uint32_t = ::uint32_t;
using uint64_t = ::uint64_t;

}

#endif