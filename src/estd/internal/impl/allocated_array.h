#pragma once

#include "../../allocators/handle_desc.h"

namespace estd { namespace internal { namespace impl {


template <class TAllocator, class Policy>
struct allocated_array : estd::handle_descriptor<TAllocator>
{
    typedef estd::handle_descriptor<TAllocator> base_type;
    typedef Policy policy_type;

    ESTD_CPP_DEFAULT_CTOR(allocated_array)
    ESTD_CPP_FORWARDING_CTOR(allocated_array)
};


}}}
