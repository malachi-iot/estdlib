#pragma once

namespace estd { namespace internal {

struct rtto_modes
{
    enum modes
    {
        COPY,
        MOVE,
        MOVE_AND_DESTROY,
        DELETE,
        SIZE,

        // +++ EXPERIMENTAL
        CREATE,
        GET_METADATA,
        COPY_AND_SWAP,
        MOVE_AND_SWAP,
        GET_INVOKE,
        // --- EXPERIMENTAL
    };

    /// generally returns POSIX errno.h codes except for:
    /// SIZE which returns type size
    using utility_type = int (*)(modes, void*, int, void*);
};


}}
