#pragma once

#include "iterators/list.h"
#include "traits/node_list.h"
#include "forward_list.h"

namespace estd {

template<class T, class TNode = T,
         class TAllocator = experimental_std_allocator<TNode>,
         class TNodeTraits = node_traits<TNode, TAllocator, nothing_allocator<T> > >
class list
{
};

}
