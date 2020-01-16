#pragma once

#ifdef FEATURE_CPP_VARIADIC
// FIX: Resolve name collision with other FEATURE_ESTD_CHRONO
// this one guards against compilation against pre c++11 scenarios
#define FEATURE_ESTD_CHRONO2
#endif

#include "port/mutex.h"
#ifdef FEATURE_CPP_VARIADIC
#include "chrono.h"
#endif
#include "internal/value_evaporator.h"

namespace estd {

// do not acquire ownership of the mutex
struct defer_lock_t
{
#ifdef FEATURE_CPP_DEFAULT_CTOR
    explicit defer_lock_t() = default;
#endif
};

// try to acquire ownership of the mutex without blocking
struct try_to_lock_t
{
#ifdef FEATURE_CPP_DEFAULT_CTOR
    explicit try_to_lock_t() = default;
#endif
};


// assume the calling thread already has ownership of the mutex
struct adopt_lock_t
{
#ifdef FEATURE_CPP_DEFAULT_CTOR
    explicit adopt_lock_t() = default;
#endif
};


// TODO: Consider deviating this from normal unique_lock by using an inline mutex
// or perhaps we make a layer1::unique_lock for that purpose
// NOTE: value_evaporator extra experimental code ironically disappeared...
template <
        class TMutex,
        class TBase = experimental::instance_provider<TMutex*> >
class unique_lock : protected TBase
{
    typedef TBase base_type;

public:
    typedef TMutex mutex_type;

    mutex_type* mutex() const { return base_type::value(); }

    mutex_type* release()
    {
        mutex_type* temp = mutex();
        base_type::value(NULLPTR);
        return temp;
    }

    void lock() { mutex()->lock(); }

    void unlock() { mutex()->unlock(); }

#ifdef FEATURE_ESTD_CHRONO2
    template< class Rep, class Period >
    bool try_lock_for( const estd::chrono::duration<Rep,Period>& timeout_duration )
    {
        return mutex()->try_lock_for(timeout_duration);
    }
#endif

    unique_lock() : base_type(NULLPTR) {}

    explicit unique_lock(mutex_type& m) : base_type(&m)
    {
        m.lock();
    }

    unique_lock(mutex_type& m, defer_lock_t) : base_type(&m) {}
    unique_lock(mutex_type& m, adopt_lock_t) : base_type(&m) {}

    unique_lock( mutex_type& m, try_to_lock_t) : base_type(&m)
    {
        m.try_lock();
    }

#ifdef FEATURE_ESTD_CHRONO2
    template< class Rep, class Period >
    unique_lock( mutex_type& m, 
             const estd::chrono::duration<Rep,Period>& timeout_duration ) :
            base_type(&m)
    {
        m.try_lock_for(timeout_duration);
    }
#endif
};



template <class Mutex>
class lock_guard
{
    typedef Mutex mutex_type;

    // needs to be a reference so that mutex ctor/dtor
    // doesn't accidentally spin up/spin down a 2nd mutex
    // TODO: Look into RVO's pass-in relative to see
    // if that indeed is going to happen
    mutex_type& m;

public:
#ifdef FEATURE_CPP_DELETE_CTOR
    lock_guard(const lock_guard&) = delete;
#endif
    explicit lock_guard(mutex_type& m) : m(m)
    {
        m.lock();
    }

    ~lock_guard()
    {
        m.unlock();
    }
};


}