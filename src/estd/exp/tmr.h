#pragma once

#include "../internal/platform.h"
#include "../type_traits.h"

// template memory resource
// a more compile-time version of pmr and scoped_memory tools

namespace estd { namespace experimental {

// TODO: put this into proper 'scoped_allocator' header
// TODO: if we can adapt this to pre-c++11 (pre variadic) then we can expressly
// lean on it to solve the linked list node allocation debacle, as well as for
// more traditional vector allocations
// https://stackoverflow.com/questions/22148258/what-is-the-purpose-of-stdscoped-allocator-adapter
// NOTE std:: implementation of this won't be OuterAlloc& friendly.  Our lib tends to be Allocator&
// friendly, but we might elect to skip that capability for scoped_allocator_adapter as it's
// a small (though present) edge case for OuterAlloc&
// NOTE nested/scoped allocator might not be precisely the right tool since for example our vector
// allocating a vector node, then allocating the value itself, is more of a sibling relationship
// than a parent child (though still leans towards parent being the node)
#ifdef FEATURE_CPP_VARIADIC
template <class... >
class scoped_allocator_adaptor {};


/*
template<class OuterAlloc>
class scoped_allocator_adaptor<OuterAlloc> : public OuterAlloc
{
public:
    typedef OuterAlloc outer_allocator_type;
}; */


template <class OuterAlloc>
class scoped_allocator_adaptor<OuterAlloc> : public OuterAlloc
{
public:
    typedef OuterAlloc outer_allocator_type;
    typedef scoped_allocator_adaptor<OuterAlloc> inner_allocator_type;

    inner_allocator_type& inner_allocator()
    {
        return *this;
    }
};

template<class OuterAlloc, class... InnerAllocs>
class scoped_allocator_adaptor<OuterAlloc, InnerAllocs...> : public OuterAlloc
{
public:
    typedef OuterAlloc outer_allocator_type;
    typedef scoped_allocator_adaptor<InnerAllocs...> inner_allocator_type;

protected:
    inner_allocator_type inner_allocators;

public:
    inner_allocator_type& inner_allocator()
    {
        return inner_allocators;
    }
};
#endif

namespace tmr {



}

}}
