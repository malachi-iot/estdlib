#pragma once

#include "../../allocators/handle_desc.h"

namespace estd { namespace internal { namespace impl {


template <class TAllocator, class Policy = monostate>
struct allocated_array : estd::handle_descriptor<TAllocator>
{
    typedef estd::handle_descriptor<TAllocator> base_type;
    typedef typename base_type::allocator_traits allocator_traits;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;
    typedef Policy policy_type;

    ESTD_CPP_DEFAULT_CTOR(allocated_array)
    ESTD_CPP_FORWARDING_CTOR(allocated_array)
};


}}}
