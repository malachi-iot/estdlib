#pragma once

namespace estd { namespace units { inline namespace v1 {

namespace detail {

enum options
{
    none,

    default_prohibited  = 0x01,         ///< Default
    value_initialized   = 0x02,
    default_initialized = 0x03,

    default_mask        = 0x03,

    // 04MAR26 - Not to be confused with only permissive construction, this is also the other direction converting unit -> intrinsic
    implicit_rep        = 0x04,         // Permit implicit conversion to and from 'rep' type (NOT USED YET)

    // As per https://github.com/malachi-iot/estdlib/issues/183 we very probably want to transition this flag
    // to focus itself on period rather than rep
    permissive          = 0x08,         // Permit conditions which risk precision loss (ALPHA quality, may phase out)
};

}

}}}
