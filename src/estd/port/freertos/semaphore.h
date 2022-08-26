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

    template< class Rep, class Period >
    bool try_acquire_for(const estd::chrono::duration<Rep,Period>& timeout_duration)
    {
        estd::chrono::freertos_clock::duration d = timeout_duration;

        return s.take(d.count());
    }

    void release()
    {
        s.give();
    }
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
};
#endif

// NOTE: Deviate from spec in that we have implicit '0' for desired,
// as well as unsigned instead of ptrdiff_t

template<unsigned max>
class counting_semaphore<max, false> : public internal::semaphore
{
public:
    counting_semaphore(unsigned desired = 0) :
        internal::semaphore(wrapped::create(counting_tag(), max, desired))
    {}
};

template <>
class counting_semaphore<1, false> : public internal::semaphore
{
public:
    counting_semaphore() :
        internal::semaphore(create(binary_tag()))
    {}
};


template<unsigned max>
class counting_semaphore<max, true> : public internal::semaphore
{
    StaticSemaphore_t storage;

public:
    counting_semaphore(unsigned desired = 0) :
        internal::semaphore(
            wrapped::create(counting_tag(), max, desired, &storage))
    {}
};

}

#if FEATURE_ESTD_FREERTOS_THREAD
template <unsigned max>
using counting_semaphore = freertos::counting_semaphore<max, false>;
#endif

}