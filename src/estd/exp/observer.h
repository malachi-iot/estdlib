#pragma once

#include "../internal/platform.h"
#include "../utility.h" // for declval
#include "../type_traits.h"

// inspired by https://github.com/ETLCPP/etl/blob/master/include/etl/observer.h

namespace estd { namespace experimental {

namespace internal {

// FEATURE_ESTD_EXPLICIT_OBSERVER is helpful for enforcing < c++11 compliance or for
// troubleshooting lost notifications (due to improper overloading of on_notify)
#if defined(FEATURE_CPP_DECLTYPE) && !defined(FEATURE_ESTD_EXPLICIT_OBSERVER)
// https://stackoverflow.com/questions/23987925/using-sfinae-to-select-function-based-on-whether-a-particular-overload-of-a-func
// Used so that on_notify calls are optional
// fallback one for when we just can't match the on_notify
template <class TObserver, class TNotifier>
static auto notify_helper(TObserver& observer, const TNotifier& n, int) -> bool
{
    return true;
}


// fallback for invocation with context where no on_notify is present
template <class TObserver, class TNotifier, class TContext>
static auto notify_helper(TObserver& observer, const TNotifier& n, TContext&, int) -> bool
{
    return true;
}

// bool gives this one precedence, since we call with (n, true)
template <class TObserver, class TNotifier>
static auto notify_helper(TObserver& observer, const TNotifier& n, bool)
    -> decltype(std::declval<TObserver>().on_notify(n), void(), bool{})
{
    observer.on_notify(n);

    return true;
}

template <class TObserver, class TNotifier, class TContext>
static auto notify_helper(TObserver& observer, const TNotifier& n, TContext& context, bool)
    -> decltype(std::declval<TObserver>().on_notify(n), void(), bool{})
{
    observer.on_notify(n);

    return true;
}

// bool gives this one precedence, since we call with (n, true)
template <class TObserver, class TNotifier, class TContext>
static auto notify_helper(TObserver& observer, const TNotifier& n, TContext& context, bool)
    -> decltype(std::declval<TObserver>().on_notify(n, context), void(), bool{})
{
    observer.on_notify(n, context);

    return true;
}

// stateless ones.  Probably we could use above ones but this way we can avoid
// inline construction of an entity altogether
// fallback one for when we just can't match the on_notify
template <class TObserver, class TNotifier>
static auto notify_helper(const TNotifier& n, int) -> bool
{
    return true;
}

// fallback for invocation with context where no on_notify is present
template <class TObserver, class TNotifier, class TContext>
static auto notify_helper(const TNotifier& n, TContext&, int) -> bool
{
    return true;
}

// bool gives this one precedence, since we call with (n, true)
template <class TObserver, class TNotifier>
static auto notify_helper(const TNotifier& n, bool)
    -> decltype(TObserver::on_notify(n), void(), bool{})
{
    TObserver::on_notify(n);

    return true;
}


// bool gives this one precedence, since we call with (n, true)
template <class TObserver, class TNotifier, class TContext>
static auto notify_helper(const TNotifier& n, TContext& context, bool)
    -> decltype(TObserver::on_notify(n), void(), bool{})
{
    TObserver::on_notify(n);

    return true;
}

// bool gives this one precedence, since we call with (n, true)
template <class TObserver, class TNotifier, class TContext>
static auto notify_helper(const TNotifier& n, TContext& context, bool)
    -> decltype(TObserver::on_notify(n, context), void(), bool{})
{
    TObserver::on_notify(n, context);

    return true;
}
#else
template <class TObserver, class TNotifier>
static void notify_helper(TObserver& observer, const TNotifier& n, bool)
{
    observer.on_notify(n);
}

template <class TObserver, class TNotifier>
static void notify_helper(const TNotifier& n, bool)
{
    TObserver::on_notify(n);
}
#endif


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
    static void notify(const TNotification&)
    {
    }

    template <class TNotification, class TContext>
    static void notify(const TNotification&, TContext&)
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
        internal::notify_helper<TObserver, TNotification>(n, true);
        base_t::notify(n);
    }

    template <class TNotification, class TContext>
    static void notify(const TNotification& n, TContext& context)
    {
        internal::notify_helper<TObserver, TNotification>(n, context, true);
        base_t::notify(n, context);
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

// benefit over a std::vector style approach in that we don't need virtual functions

template <class ...>
class subject;


template <>
class subject<>
{
public:
    template <class TNotifier>
    void notify(const TNotifier& n) const {}

    template <class TNotifier, class TContext>
    void notify(const TNotifier &, TContext&) const {}
};


template <class TObserver, class ...TObservers>
class subject<TObserver&, TObservers...> : public subject<TObservers...>
{
    typedef subject<TObservers...> base_t;

protected:
    TObserver& observer;
public:
    constexpr subject(TObserver& observer, TObservers&&...observers) :
            base_t(std::forward<TObservers>(observers)...),
            observer(observer)
    {}


    template <class TNotifier>
    void notify(const TNotifier &n)
    {
        internal::notify_helper(observer, n, true);

        base_t::notify(n);
    }


    template <class TNotifier, class TContext>
    void notify(const TNotifier &n, TContext& context)
    {
        internal::notify_helper(observer, n, context, true);

        base_t::notify(n, context);
    }
};


template <class TObserver, class ...TObservers>
class subject<TObserver, TObservers...> : public subject<TObservers...>
{
    typedef subject<TObservers...> base_t;

protected:
    typedef typename std::remove_reference<TObserver>::type observer_t;

    observer_t observer;
public:
    constexpr subject(TObserver&& observer, TObservers&&...observers) :
            base_t(std::forward<TObservers>(observers)...),
            observer(std::move(observer))
    {}

    // for implicit-constructed version
    constexpr subject(TObservers&&...observers) :
            base_t(std::forward<TObservers>(observers)...)
    {}

    template <class TNotifier>
    void notify(const TNotifier &n)
    {
#ifdef UNIT_TESTING
        int sz = sizeof(TObserver);
#endif

        internal::notify_helper(observer, n, true);

        base_t::notify(n);
    }

    template <class TNotifier, class TContext>
    void notify(const TNotifier &n, TContext& context)
    {
        internal::notify_helper(observer, n, context, true);

        base_t::notify(n, context);
    }
};


template <class ...TObservers>
subject<TObservers&&...> make_subject(TObservers&&...observers)
{
    return subject<TObservers&&...>(
            std::forward<TObservers>(observers)...);
}


template <class ...TObservers>
constexpr subject<TObservers&&...> make_subject_const(TObservers&&...observers)
{
    return subject<TObservers&&...>(
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

// https://stackoverflow.com/questions/22968182/is-it-possible-to-typedef-a-parameter-pack
// however doesnt work out for our mostly-not-parameter-pack approach
template<typename... Args>
struct pack { };

template <class ...>
class observer_abstract;

template <>
class observer_abstract<>
{
public:
};

// making discrete sets instead of using parameter pack/variadic because
// C++ makes a new virtual table per class, and that chews up some valuable memory space
template <class TNotification>
class observer_abstract<TNotification>
{
public:
    using args = pack<TNotification>;

    typedef TNotification notification_1_type;

    virtual void on_notify(const TNotification&) = 0;
};


template <class TNotification1, class TNotification2>
class observer_abstract<TNotification1, TNotification2>
{
public:
    using args = pack<TNotification1, TNotification2>;

    typedef TNotification1 notification_1_type;
    typedef TNotification2 notification_2_type;

    virtual void on_notify(const TNotification1&) = 0;
    virtual void on_notify(const TNotification2&) = 0;
};


template <class TNotification1, class TNotification2, class TNotification3>
class observer_abstract<TNotification1, TNotification2, TNotification3>
{
public:
    using args = pack<TNotification1, TNotification2, TNotification3>;

    typedef TNotification1 notification_1_type;
    typedef TNotification2 notification_2_type;
    typedef TNotification3 notification_3_type;

    virtual void on_notify(const TNotification1&) = 0;
    virtual void on_notify(const TNotification2&) = 0;
    virtual void on_notify(const TNotification3&) = 0;
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
    observer_proxy(TSubject& s) : subject(s) {}

    observer_proxy(TSubject&& s) : subject(std::move(s)) {}

    observer_proxy() {}
};

template <class TSubject, class TNotification>
class observer_proxy<TSubject, TNotification> :
        public observer_abstract<TNotification>,
        public observer_proxy<TSubject>
{
    typedef observer_proxy<TSubject> base_t;

public:
    observer_proxy(TSubject& s) : base_t(s) {}

    observer_proxy(TSubject&& s) : base_t(std::move(s)) {}

    observer_proxy() {}

    virtual void on_notify(const TNotification& n) override
    {
        base_t::subject.notify(n);
    }
};



template <class TSubject, class TNotification1, class TNotification2>
class observer_proxy<TSubject, TNotification1, TNotification2> :
        public observer_abstract<TNotification1, TNotification2>,
        public observer_proxy<TSubject>
{
    typedef observer_proxy<TSubject> base_t;

public:
    observer_proxy(TSubject& s) : base_t(s) {}

    observer_proxy(TSubject&& s) : base_t(std::move(s)) {}

    observer_proxy() {}

    virtual void on_notify(const TNotification1& n) override
    {
        base_t::subject.notify(n);
    }

    virtual void on_notify(const TNotification2& n) override
    {
        base_t::subject.notify(n);
    }
};

// pretty lame how explicit these are
template <class TSubject, class observer_abstract >
observer_proxy<TSubject, typename observer_abstract::notification_1_type> make_proxy()
{
    observer_proxy<TSubject, typename observer_abstract::notification_1_type> p;

    return p;
}


template <class TSubject, class observer_abstract >
observer_proxy<TSubject,
    typename observer_abstract::notification_1_type,
    typename observer_abstract::notification_2_type> make_proxy_2()
{
    observer_proxy<TSubject,
            typename observer_abstract::notification_1_type,
            typename observer_abstract::notification_2_type> p;

    return p;
}


#endif



struct void_subject
{
    template <class TNotifier>
    void notify(const TNotifier&) {}

    template <class TNotifier, class TContext>
    void notify(const TNotifier&, TContext&) {}
};

}}
