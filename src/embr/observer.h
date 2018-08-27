#pragma once

#include "../estd/exp/observer.h"
#include "../estd/tuple.h"
#include "../estd/functional.h"

#ifdef FEATURE_CPP_VARIADIC

// NOTE: Not otherwise labeled as such but this file's naming is experimental
namespace embr {

namespace internal {

template <class ...TObservers>
class tuple_base
{
protected:
    typedef estd::tuple<TObservers...> tuple_type;

    tuple_type observers;

    template <int index, class TEvent>
    void _notify_helper(const TEvent& e)
    {
        estd::tuple_element_t<index, tuple_type>& observer =
                estd::get<index>(observers);

        estd::experimental::internal::notify_helper(observer, e, true);
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

    template <int index, class TEvent>
    void _notify_helper(const TEvent& e)
    {
        // allocate a purely temporary observer, as this has
        // been explicitly set up as stateless
        estd::tuple_element_t<index, tuple_type> observer;

        // SFINAE magic to call best matching on_notify function
        estd::experimental::internal::notify_helper(
                    observer,
                    e, true);
    }
};

template <class TBase, class ...TObservers>
class subject : TBase
{
    typedef TBase base_type;
    typedef typename base_type::tuple_type tuple_type;

    // using slightly clumsy index >= 0 so that Qt tabbing doesn't get confused
    template <int index, class TEvent,
              class TEnabled = typename estd::enable_if <!(index >= 0), bool>::type >
    void notify_helper(const TEvent&) const
    {

    }

    template <int index, class TEvent,
              class TEnabled = typename estd::enable_if<(index >= 0), void>::type >
    void notify_helper(const TEvent& e, bool = true)
    {
        base_type::template _notify_helper<index>(e);

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

namespace layer0 {

#ifdef FEATURE_CPP_ALIASTEMPLATE
template <class ...TObservers>
using subject = internal::subject<
    internal::stateless_base<TObservers...>,
    TObservers...>;

#endif

}
namespace layer1 {

#ifdef FEATURE_CPP_ALIASTEMPLATE
template <class ...TObservers>
using subject = internal::subject<
    internal::tuple_base<TObservers...>,
    TObservers...>;
#else
template <class ...TObservers>
class subject : internal::subject<internal::tuple_base<TObservers...> >
{
public:
};
#endif

template <class ...TObservers>
subject<TObservers&&...> make_subject(TObservers&&...observers)
{
    return subject<TObservers&&...>(
            std::forward<TObservers>(observers)...);
}



}

}

#endif

