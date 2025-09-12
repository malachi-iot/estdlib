#pragma once

#include "../../../flags.h"

namespace estd { namespace internal {

enum class queue_options
{
    bare        = 0x0001,   ///< No knowledge of empty, full, count or rollover
    sentinel    = 0x0002,
    flagged     = 0x0003,
    counter     = 0x0004,
    mask        = 0x0007,

    atomic      = 0x0008,

    // force trivial behavior i.e. calls default constructor
    trivial     = 0x0010,
    // reject-on-full mode (needed for full lock-free behavior)
    no_rollover = 0x0020,
    // EXPERIMENTAL - inhibit would-be exceptions/invalid condition checks
    // similar to c++26 contracts as per
    // https://en.cppreference.com/w/cpp/container/deque/pop_back.html
    // https://en.cppreference.com/w/cpp/language/contracts.html
    strict      = 0x0040,

    default_opt = flagged
};

ESTD_FLAGS(queue_options)

}}