#pragma once

#include "port/mutex.h"
#include "internal/value_evaporator.h"

namespace estd {

// do not acquire ownership of the mutex
struct defer_lock_t { explicit defer_lock_t() = default; };

// try to acquire ownership of the mutex without blocking
struct try_to_lock_t { explicit try_to_lock_t() = default; };

// assume the calling thread already has ownership of the mutex
struct adopt_lock_t { explicit adopt_lock_t() = default; };

// TODO: Consider deviating this from normal unique_lock by using an inline mutex
// or perhaps we make a layer1::unique_lock for that purpose
// NOTE: value_evaporator extra experimental code ironically disappeared...
template <class TMutex, class TBase = void>
class unique_lock
{
    TMutex* _mutex;

public:
    typedef TMutex mutex_type;

    mutex_type* mutex() const { return _mutex; }

    mutex_type* release()
    {
        mutex_type* temp = _mutex;
        _mutex = NULLPTR;
        return temp;
    }

    void lock() { mutex()->lock(); }

    void unlock() { mutex()->unlock(); }

    template< class Rep, class Period >
    bool try_lock_for( const estd::chrono::duration<Rep,Period>& timeout_duration )
    {
        return mutex()->try_lock_for(timeout_duration);
    }

    //unique_lock() : _mutex(NULLPTR) {}

    explicit unique_lock(mutex_type& m) : _mutex(&m) 
    {
        m.lock();
    }

    unique_lock(mutex_type& m, defer_lock_t) : _mutex(&m) {}
    unique_lock(mutex_type& m, adopt_lock_t) : _mutex(&m) {}

    unique_lock( mutex_type& m, try_to_lock_t) : _mutex(&m)
    {
        m.try_lock();
    }

    template< class Rep, class Period >
    unique_lock( mutex_type& m, 
             const estd::chrono::duration<Rep,Period>& timeout_duration ) :
             _mutex(&m)
    {
        m.try_lock_for(timeout_duration);
    }
};

}