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

    template <int index, class TEvent,
              class TEnabled = typename estd::enable_if<index < 0, bool>::type >
    void notify_helper(const TEvent& e)
    {

    }

    template <int index, class TEvent,
              class TEnabled = typename estd::enable_if<index >= 0, void>::type >
    void notify_helper(const TEvent& e, bool = true)
    {
        const auto& observer =
                estd::get<index>(observers);

        estd::experimental::internal::notify_helper(observer, e, true);

        if(index > 0)
            notify_helper<index - 1>(e);
    }

public:
    constexpr subject(TObservers&&...observers) :
            observers(std::forward<TObservers>(observers)...)
    {}

    subject() {}

    template <class TEvent>
    void notify(const TEvent& e)
    {
        notify_helper<estd::tuple_size<decltype(observers)>::value - 1>(e);
    }
};

}

}
