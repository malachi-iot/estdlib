#pragma once

#include "internal/queue/circular.h"
#include "internal/queue/legacy.h"

namespace estd {

#if FEATURE_ESTD_GH144
namespace layer1 {

template <class T, size_t N>
using deque = ring<T, N, ring_options::hardened | ring_options::flagged>;

}
#endif

}
