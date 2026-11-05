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

// TODO: Do up FEATURE_STD_MUTEX_ALIAS for these defer_lock_t and friends

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
        class Mutex,
        class Base = experimental::instance_provider<Mutex*> >
class unique_lock : protected Base
{
    using base_type = Base;

public:
    typedef Mutex mutex_type;

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

// 05NOV26 MB - estd::internal::concepts is dangerous, because all our
// ESTD_CPP_CONCEPTS(concepts::xxx) risk getting broken
//namespace internal {

#if __cpp_concepts
// DEBT: Belongs elsewhere
namespace concepts {

template <class T>
concept Mutex = requires(T m)
{
    m.lock();
    m.unlock();
};

}

#endif

namespace internal {

template <class Mutex, bool is_empty = is_empty<Mutex>::value>
class lock_guard;

struct noop_mutex
{
    static constexpr bool lock() { return true; }
    static constexpr bool try_lock() { return true; }

    static ESTD_CPP_CONSTEXPR(14) void unlock() { }
};


// 03NOV26 MB DEBT: Consolidate with EBO/evaporators
template <class Mutex>
class lock_guard<Mutex, true>
{
protected:
    using arg_type = Mutex;

    ESTD_CPP_CONSTEXPR(14) explicit lock_guard(Mutex)
    {
        Mutex{}.lock();
    }

    ESTD_CPP_CONSTEXPR(14) ~lock_guard() { Mutex{}.unlock(); }
};

template <class Mutex>
class lock_guard<Mutex, false>
{
protected:
    using arg_type =  Mutex&;

    // needs to be a reference so that mutex ctor/dtor
    // doesn't accidentally spin up/spin down a 2nd mutex
    // TODO: Look into RVO's pass-in relative to see
    // if that indeed is going to happen
    arg_type mutex_;

    ESTD_CPP_CONSTEXPR(14) explicit lock_guard(arg_type m) : mutex_(m)
    {
        m.lock();
    }

    ESTD_CPP_CONSTEXPR(14) ~lock_guard() { mutex_.unlock(); }
};

}

// Not doing a 'using' just because it's friendlier to debugger to wrap it
// and not hard to do so
template <ESTD_CPP_CONCEPT(concepts::Mutex) Mutex>
class lock_guard : public internal::lock_guard<Mutex>
{
    using base_type = internal::lock_guard<Mutex>;
    using typename base_type::arg_type;

public:
    using mutex_type = Mutex;

    lock_guard(const lock_guard&) = delete;

    ESTD_CPP_CONSTEXPR(14) explicit lock_guard(arg_type m) :
        base_type(m)
    {
    }

    // TODO: Add in adopt_lock_t flavor
};


}