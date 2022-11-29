// Based on https://raspberrypi.github.io/pico-sdk-doxygen/group__sleep.html
// NOTE: On hold because regular std::system_clock and friends are present
#pragma once

namespace estd {

namespace chrono {

struct pico_clock
{
    typedef uint64_t rep;
};

}

}