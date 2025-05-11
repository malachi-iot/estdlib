#pragma once

#include "../../allocators/handle_desc.h"

namespace estd { namespace internal { namespace impl {


// Mainly a place to park policy_type
template <class Allocator, class Policy>
struct allocated_array : estd::handle_descriptor<Allocator>
{
    using base_type = estd::handle_descriptor<Allocator>;
    using policy_type = Policy;

    ESTD_CPP_DEFAULT_CTOR(allocated_array)
    ESTD_CPP_FORWARDING_CTOR(allocated_array)
};


}}}
