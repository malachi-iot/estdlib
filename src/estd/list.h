#pragma once

#include "iterators/list.h"
#include "traits/node_list.h"
#include "forward_list.h"

namespace estd {

template<class T, class TNodeTraits = node_traits<T > >
class list
{
};

}