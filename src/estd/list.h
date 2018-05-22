#pragma once

#include "iterators/list.h"
#include "traits/node_list.h"
#include "forward_list.h"

namespace estd {

template<class T, class TNode = T,
         class TAllocator = experimental_std_allocator<TNode>,
         class TNodeTraits = node_traits<TNode, TAllocator, nothing_allocator<T> > >
class list : public internal::linkedlist_base<T, TNodeTraits>
{
    typedef internal::linkedlist_base<T, TNodeTraits> base_t;
    typedef typename base_t::allocator_t allocator_t;
public:
    list(allocator_t* allocator = NULLPTR) : base_t(allocator) {}
};

}
