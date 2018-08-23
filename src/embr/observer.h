#pragma once

#include "../estd/exp/observer.h"
#include "../estd/tuple.h"

// NOTE: Not otherwise labeled as such but this file's naming is experimental
namespace embr {

#ifdef FEATURE_CPP_VARIADIC
#endif

namespace layer1 {

template <class ...TObservers>
class subject
{
    typedef estd::tuple<TObservers...> tuple_type;

    tuple_type observers;

    template <int index, class TEvent,
              class TEnabled = typename estd::enable_if<index < 0, bool>::type >
    void notify_helper(const TEvent&)
    {

    }

    template <int index, class TEvent,
              class TEnabled = typename estd::enable_if<index >= 0, void>::type >
    void notify_helper(const TEvent& e, bool = true)
    {
        estd::tuple_element_t<index, tuple_type>& observer =
                estd::get<index>(observers);

        // SFINAE magic to call best matching on_notify function
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
