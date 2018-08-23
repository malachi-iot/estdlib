#pragma once

#include "../estd/exp/observer.h"
#include "../estd/tuple.h"

// NOTE: Not otherwise labeled as such but this file's naming is experimental
namespace embr {

#ifdef FEATURE_CPP_VARIADIC
#endif

namespace layer1 {

namespace internal {

template <class ...TObservers>
class tuple_base
{
protected:
    typedef estd::tuple<TObservers...> tuple_type;

    tuple_type observers;

    template <int index>
    estd::tuple_element_t<index, tuple_type>& get()
    {
        return estd::get<index>(observers);
    }

    tuple_base(TObservers&&...observers) :
        observers(std::forward<TObservers>(observers)...)
    {}

    tuple_base() {}

};

// slightly abuses tuple type.  We pretend we have a tuple
template <class ...TObservers>
class stateless_base
{
protected:
    typedef estd::tuple<TObservers...> tuple_type;

    template <int index>
    estd::tuple_element_t<index, tuple_type> get()
    {
        return estd::tuple_element_t<index, tuple_type> {};
    }
};

}

template <class ...TObservers>
class subject : internal::tuple_base<TObservers...>
{
    typedef internal::tuple_base<TObservers...> base_type;
    typedef typename base_type::tuple_type tuple_type;

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
                base_type::template get<index>();

        // SFINAE magic to call best matching on_notify function
        estd::experimental::internal::notify_helper(observer, e, true);

        if(index > 0)
            notify_helper<index - 1>(e);
    }

public:
    constexpr subject(TObservers&&...observers) :
            base_type(std::forward<TObservers>(observers)...)
    {}

    subject() {}

    template <class TEvent>
    void notify(const TEvent& e)
    {
        notify_helper<sizeof... (TObservers) - 1>(e);
    }
};

}

}
