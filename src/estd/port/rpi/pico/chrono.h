// Based on https://raspberrypi.github.io/pico-sdk-doxygen/group__sleep.html
// NOTE: On hold because regular std::system_clock and friends are present
#pragma once

#include <chrono>

namespace estd {

namespace chrono {

namespace experimental {

struct pico_clock
{
    typedef uint64_t rep;
};

}

typedef std::chrono::system_clock system_clock;
typedef std::chrono::steady_clock steady_clock;

}

}