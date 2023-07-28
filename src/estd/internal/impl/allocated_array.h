#pragma once

#include "../../allocators/handle_desc.h"

namespace estd { namespace internal { namespace impl {

template <class TAllocator, class Policy = monostate>
struct allocator_array;

template <class TAllocator, class Policy = monostate>
struct allocated_array : estd::handle_descriptor<TAllocator>
{
    typedef estd::handle_descriptor<TAllocator> base_type;
    typedef base_type base_t;
    typedef typename base_t::allocator_type allocator_type;
    typedef typename base_t::allocator_traits allocator_traits;
    typedef typename allocator_traits::handle_with_offset handle_with_offset;
    typedef Policy policy_type;

    typedef typename base_t::size_type size_type;

    ESTD_CPP_CONSTEXPR_RET size_type max_size() const
    { return base_type::get_allocator().max_size(); }

    ESTD_CPP_DEFAULT_CTOR(allocated_array)

    template <class TAllocatorParam>
    allocated_array(TAllocatorParam& p) : base_type(p) {}

    handle_with_offset offset(size_type pos) const
    {
        return base_type::get_allocator().offset(base_t::handle(), pos);
    }
};


}}}
