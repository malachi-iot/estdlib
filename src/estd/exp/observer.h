#pragma once

// inspired by https://github.com/ETLCPP/etl/blob/master/include/etl/observer.h

namespace estd { namespace experimental {

namespace internal {

template <class TContainer>
class subject
{
    typedef typename TContainer::value_type observer_type;
public:

};

}

}}