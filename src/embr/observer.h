#pragma once

#include "../estd/exp/observer.h"
#include "../estd/tuple.h"

// NOTE: Not otherwise labeled as such but this file's naming is experimental
namespace embr {

#ifdef FEATURE_CPP_VARIADIC
#endif

namespace layer0 {

template <class ...TObservers>
class subject
{
    estd::tuple<TObservers...> observers;

public:
    constexpr subject(TObservers&&...observers) :
            observers(std::forward<TObservers>(observers)...)
    {}

    subject() {}

    template <class TEvent>
    void notify(const TEvent&)
    {

    }
};

}

}
