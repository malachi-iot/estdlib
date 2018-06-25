#pragma once

#include "../internal/platform.h"
#include <type_traits>
#include <functional>

#include "../type_traits.h"

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



template <>
class stateless_subject<>
{
public:
    template <class TNotification>
    static void notify(const TNotification& n)
    {
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

// since this doesn't take both TObserver and TAllocator, for now consider this an internal
// class
// remember observer_type in this context must implement virtual on_notify calls
// this particuar class most strongly resembles the ETL subject/observer implementation
template <class TContainer>
class container_subject
{
    typedef typename estd::remove_reference<TContainer>::type container_type;

    typedef typename container_type::value_type observer_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;

    container_type c;

public:
    template <class TNotification>
    void notify(const TNotification& n)
    {
        iterator i = c.begin();

        // FIX: because reference_wrapper doesn't yet work with estd::vector, demanding
        // pointer version of things
        for(;i != c.cend(); i++)
            (*i)->on_notify(n);
    }
};



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


template <>
class subject2<>
{
public:
    template <class TNotifier>
    void notify(const TNotifier& n) const {}
};


template <class TObserver, class ...TObservers>
class subject2<TObserver&, TObservers...> : public subject2<TObservers...>
{
    typedef subject2<TObservers...> base_t;

protected:
    TObserver& observer;
public:
    constexpr subject2(TObserver& observer, TObservers&&...observers) :
            base_t(std::forward<TObservers>(observers)...),
            observer(observer)
    {}

    template <class TNotifier>
    void notify(const TNotifier& n)
    {
        observer.on_notify(n);

        base_t::notify(n);
    }
};


template <class TObserver, class ...TObservers>
class subject2<TObserver, TObservers...> : public subject2<TObservers...>
{
    typedef subject2<TObservers...> base_t;

protected:
    TObserver observer;
public:
    constexpr subject2(TObserver&& observer, TObservers&&...observers) :
            base_t(std::forward<TObservers>(observers)...),
            observer(std::move(observer))
    {}

    // for implicit-constructed version
    constexpr subject2(TObservers&&...observers) :
            base_t(std::forward<TObservers>(observers)...)
    {}

    template <class TNotifier>
    void notify(const TNotifier& n)
    {
        observer.on_notify(n);

        base_t::notify(n);
    }
};


template <class ...TObservers>
subject2<TObservers&&...> make_subject(TObservers&&...observers)
{
    return subject2<TObservers&&...>(
            std::forward<TObservers>(observers)...);
}


template <class ...TObservers>
constexpr subject2<TObservers&&...> make_subject_const(TObservers&&...observers)
{
    return subject2<TObservers&&...>(
            std::forward<TObservers>(observers)...);
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

template <class ...>
class observer_abstract;

template <>
class observer_abstract<>
{
public:
};

template <class TNotification, class ...TNotifications>
class observer_abstract<TNotification, TNotifications...> :
        public observer_abstract<TNotifications...>
{
public:
    virtual void on_notify(const TNotification&) = 0;
};


// accepts incoming notifications and spits them back out to another
// subject.  Note this one is virtual-method specific
template <class TSubject, class ...>
class observer_proxy;

template <class TSubject>
class observer_proxy<TSubject>
{
protected:
    TSubject subject;

public:
};

template <class TSubject, class TNotification, class ...TNotifications>
class observer_proxy<TSubject, TNotification, TNotifications...> :
        public observer_abstract<TNotification, TNotifications...>,
        public observer_proxy<TSubject, TNotifications...>
{
    typedef observer_proxy<TSubject, TNotifications...> base_t;

public:
    virtual void on_notify(const TNotification& n) override
    {
        base_t::subject.notify(n);
    }
};


#endif



struct void_subject
{
    template <class TNotifier>
    void notify(const TNotifier&) {}
};

}}
