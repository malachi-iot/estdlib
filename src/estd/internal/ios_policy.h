#pragma once

namespace estd {


namespace experimental {

// NOTE: May have to make a bunch of duplicate #defines for pre C++0x compilers

struct istream_flags
{
    typedef unsigned flag_type;

    // bits 0, 1
    static constexpr flag_type non_blocking = 0,
        blocking = 1,
        runtime_blocking = 2,
        block_mask = 3,

    // bit 2
        inline_rdbuf = 0,
        traditional_rdbuf = 4,
        rdbuf_mask = 4,

        _default = 0;
};

}

}