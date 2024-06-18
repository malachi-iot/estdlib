#pragma once

#include "../iosfwd.h"
#include "../container/fwd.h"

namespace estd {

// Not ready yet
template <class Container>
class back_insert_iterator
{
    Container* container;

    using cvalue_type = typename Container::value_type;

public:
    using container_type = Container;

    constexpr explicit back_insert_iterator(Container& c) :
        container(&c)
    {}

    back_insert_iterator& operator=(const cvalue_type& value)
    {
        container->push_back(value);
    }
};

}