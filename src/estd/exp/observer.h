#pragma once

#include "../internal/platform.h"
#include <type_traits>
#include <functional>

// inspired by https://github.com/ETLCPP/etl/blob/master/include/etl/observer.h

namespace estd { namespace experimental {

namespace internal {

template <class TContainer>
class subject
{
    typedef typename TContainer::value_type observer_type;
public:

};

#ifdef FEATURE_CPP_VARIADIC
template <class ...>
class stateless_subject
{

};



template <class TObserver>
class stateless_subject<TObserver>
{
public:
    template <class TNotification>
    static void notify(const TNotification& n)
    {
        TObserver::on_notify(n);
    }
};

template <class TObserver, class ...TObservers>
class stateless_subject<TObserver, TObservers...>
{
    typedef stateless_subject<TObservers...> base_t;

public:
    template <class TNotification>
    static void notify(const TNotification& n)
    {
        base_t::notify(n);
        TObserver::on_notify(n);
    }
};
#endif


}

#ifdef FEATURE_CPP_VARIADIC
namespace layer0 {

template <class TObserver, TObserver&...observers>
class subject
{
    template <class TNotify, TObserver& o, TObserver&..._observers>
    static void notify(const TNotify& n)
    {
        o.on_notify(n);
        if(sizeof...(_observers) > 0)
            notify<TNotify, _observers...>(n);
    }

public:
    template <class TNotify>
    static void notify(const TNotify& n)
    {
        notify<TNotify, observers...>(n);
    }
};


template <class TNotification, class TObserver>
void test_notify(const TNotification& n, TObserver& observer)
{
    observer.on_notify(n);
}

template <class TNotification, class TObserver, class ...TObservers>
void test_notify(const TNotification& n, TObserver& observer, TObservers&...observers)
{
    observer.on_notify(n);
    test_notify(n, observers...);
}


// benefit over a std::vector style approach in that we don't need virtual functions

template <class ...>
class subject2;

/*
template <class TObserver>
class subject2<TObserver>
{
protected:
    TObserver last;
public:
    subject2(TObserver observer) : last(observer) {}

    template <class TNotifier>
    void notify(const TNotifier& n)
    {
        last.on_notify(n);
    }
};
*/

template <class TObserver>
class subject2<TObserver&>
{
protected:
    TObserver& last;
public:
    subject2(TObserver& observer) : last(observer) {}

    template <class TNotifier>
    void notify(const TNotifier& n)
    {
        last.on_notify(n);
    }
};


template <class TObserver>
class subject2<TObserver&&>
{
protected:
    TObserver last;
public:
    subject2(TObserver&& observer) : last(std::move(observer)) {}

    template <class TNotifier>
    void notify(const TNotifier& n)
    {
        last.on_notify(n);
    }
};



template <class TObserver, class ...TObservers>
class subject2<TObserver&, TObservers...> : public subject2<TObservers...>
{
    typedef subject2<TObservers...> base_t;

protected:
    TObserver& observer;
public:
    subject2(TObserver& observer, TObservers&...observers) :
            base_t(observers...),
            observer(observer)
    {}

    template <class TNotifier>
    void notify(const TNotifier& n)
    {
        observer.on_notify(n);

        base_t::notify(n);
    }
};


/*
template <class TObserver, class ...TObservers>
class subject2<TObserver, TObservers...> : public subject2<TObservers...>
{
    typedef subject2<TObservers...> base_t;

protected:
    TObserver observer;
public:
    subject2(TObserver&& observer, TObservers...observers) :
            base_t(observers...),
            observer(std::move(observer))
    {}

    template <class TNotifier>
    void notify(const TNotifier& n)
    {
        observer.on_notify(n);

        base_t::notify(n);
    }
}; */


template <class ...TObservers>
subject2<TObservers...> make_subject(TObservers&&...observers)
{
    return subject2<TObservers...>(std::forward<TObservers>(observers)...);
}


template <class ...TObservers>
constexpr subject2<TObservers...> make_subject_const(TObservers...observers)
{
    return subject2<TObservers...>(observers...);
}

    /*
template <class TObserver, TObserver& o, TObserver&..._>
class subject;

template <class TObserver, TObserver& observer>
class subject
{
public:
    template <class TNotification>
    static void notify(const TNotification& n)
    {
        observer.on_notify(n);
    }
};

template <class TObserver, TObserver& observer, TObserver&...observers >
class subject
{
    typedef subject<TObserver, observers...> base_t;

    template <class TNotification>
    static void _notify(const TNotification& n)
    {
        base_t::_notify(n);

        observer.on_notify(n);
    }

public:
    template <class TNotification>
    static void notify(const TNotification& n)
    {
        _notify<TNotification>(n);
    }
};
 */
}
#endif

}}
