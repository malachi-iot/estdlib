#pragma once

#include "wrapper/semphr.h"

#include "fwd.h"
#include "chrono.h"

namespace estd {
    
namespace freertos {

namespace internal {

class semaphore_base
{
protected:
    typedef wrapper::semaphore wrapped;

    const wrapper::semaphore s;

    semaphore_base(SemaphoreHandle_t s) :
        s{s} {}

    ~semaphore_base()
    {
        // From the source code:
        /* The queue could have been allocated statically or dynamically, so
         * check before attempting to free the memory. */
        // Therefore we use this for both static and dynamic allocations of
        // semaphore
        s.free();
    }

    typedef wrapped::counting_tag counting_tag;
    typedef wrapped::binary_tag binary_tag;
    typedef wrapped::mutex_tag mutex_tag;
    typedef wrapped::recursive_mutex_tag recursive_mutex_tag;

    // Helper for simple cases.  Not using parameter pack for clarity
    // and for c++03 compatibility
    template <class TTag>
    static inline SemaphoreHandle_t create(TTag tag)
    {
        return wrapped::create(tag);
    }

public:
    typedef wrapped native_handle_type;

    // NOTE: C++ spec only calls for this during mutex, but for freertos
    // it's identical so we expose it at this level
    native_handle_type native_handle() const { return s; }
};

class semaphore : public semaphore_base
{
protected:
    semaphore(SemaphoreHandle_t s) : semaphore_base(s) {}

public:
    void acquire()
    {
        s.take(portMAX_DELAY);
    }

    bool try_acquire()
    {
        return s.take(0);
    }

    template<class Rep, class Period>
    bool try_acquire_for(const estd::chrono::duration<Rep,Period>& timeout_duration)
    {
        estd::chrono::freertos_clock::duration d = timeout_duration;

        return s.take(d.count());
    }

    template<class Clock, class Duration>
    bool try_acquire_until(const estd::chrono::time_point<Clock,Duration>& timeout_time)
    {
        return s.take(timeout_time - chrono::freertos_clock::now());
    }

    void release()
    {
        s.give();
    }
};

template <unsigned m>
struct semaphore_max
{
#ifdef FEATURE_CPP_CONSTEXPR
    constexpr
#else
    inline
#endif
    static unsigned max() { return m; }
};

}

#ifndef FEATURE_CPP_ALIASTEMPLATE
template <>
class binary_semaphore<false> : public internal::semaphore
{
public:
    binary_semaphore() :
        internal::semaphore(create(binary_tag))
    {}

    static CONSTEXPR 
};
#endif

// NOTE: Deviate from spec in that we have implicit '0' for desired,
// as well as unsigned instead of ptrdiff_t
#if configSUPPORT_DYNAMIC_ALLOCATION
template<unsigned max>
struct counting_semaphore<max, false> : internal::semaphore,
    internal::semaphore_max<max>
{
    counting_semaphore(unsigned desired = 0) :
        internal::semaphore(wrapped::create(counting_tag(), max, desired))
    {}
};

template <>
struct counting_semaphore<1, false> : internal::semaphore,
    internal::semaphore_max<1>
{
    counting_semaphore() :
        internal::semaphore(create(binary_tag()))
    {}
};
#endif

#if configSUPPORT_STATIC_ALLOCATION
template<unsigned max>
struct counting_semaphore<max, true> : internal::semaphore,
    internal::semaphore_max<max>
{
private:
    StaticSemaphore_t storage;

public:
    counting_semaphore(unsigned desired = 0) :
        internal::semaphore(
            wrapped::create(counting_tag(), max, desired, &storage))
    {}
};


template<>
struct counting_semaphore<1, true> : internal::semaphore,
    internal::semaphore_max<1>
{
private:
    StaticSemaphore_t storage;

public:
    counting_semaphore() :
        internal::semaphore(
            wrapped::create(binary_tag(), &storage))
    {
    }
};
#endif


}

#if FEATURE_ESTD_FREERTOS_THREAD
template <unsigned max>
using counting_semaphore = freertos::counting_semaphore<max, false>;
#endif

}