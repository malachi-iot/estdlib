#pragma once

#include "wrapper/semphr.h"

namespace estd {
    
namespace freertos {

namespace internal {

class semaphore_base
{
protected:
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
        vSemaphoreDelete(s);
    }

public:
    typedef SemaphoreHandle_t native_handle_type;

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
        internal::semaphore(xSemaphoreCreateBinary())
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
        internal::semaphore(xSemaphoreCreateCounting(max, desired))
    {}
};

template <>
class counting_semaphore<1, false> : public internal::semaphore
{
public:
    counting_semaphore() :
        internal::semaphore(xSemaphoreCreateBinary())
    {}
};


template<unsigned max>
class counting_semaphore<max, true> : public internal::semaphore
{
    StaticSemaphore_t storage;

public:
    counting_semaphore(unsigned desired = 0) :
        internal::semaphore(
            xSemaphoreCreateCountingStatic(max, desired, &storage))
    {}
};

}

#if FEATURE_ESTD_FREERTOS_THREAD
template <unsigned max>
using counting_semaphore = freertos::counting_semaphore<max, false>;
#endif

}