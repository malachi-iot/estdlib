#pragma once

namespace estd { namespace units { inline namespace v1 {

namespace detail {

enum options
{
    none,

    default_prohibited  = 0x01,
    value_initialized   = 0x02,
    default_initialized = 0x03,

    default_mask        = 0x03,

    implicit_rep        = 0x04,         // Permit implicit conversion to and from 'rep' type (NOT USED YET)
    permissive          = 0x08,         // Permit conditions which risk precision loss (UNTESTED)
};

}

}}}
