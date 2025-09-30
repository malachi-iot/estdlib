#pragma once

#include "../../../flags.h"

namespace estd { namespace internal {

enum class queue_options
{
    bare        = 0x0001,   ///< No knowledge of empty, full, count or rollover
    sentinel    = 0x0002,
    flagged     = 0x0003,
    counter     = 0x0004,
    packed      = 0x0005,   ///< EXPERIMENTAL: flavor of 'flagged' with front, back and empty in a packed word
    mask        = 0x0007,

    atomic      = 0x0008,

    // EXPERIMENTAL
    // force trivial behavior i.e. implicitly calls default constructor, skips destructor
    // you almost definitely don't want this.  Just here for AVR GCC glitch experimentation
    trivial     = 0x0010,
    // reject-on-full mode (needed for full lock-free behavior)
    no_rollover = 0x0020,
    // EXPERIMENTAL - do true/false bounds checks instead of would-be exceptions/invalid condition checks
    // similar to c++26 contracts as per
    // https://en.cppreference.com/w/cpp/container/deque/pop_back.html
    // https://en.cppreference.com/w/cpp/language/contracts.html
    strict      = 0x0040,
    // do asserts as per above c++26 contracts
    // strict AND contract cannot coexist, specify one or the other
    contract    = 0x0080,

    default_opt = flagged
};

ESTD_FLAGS(queue_options)

}}