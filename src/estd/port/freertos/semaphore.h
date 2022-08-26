#pragma once

#include "../arch/freertos.h"

extern "C" {

#if ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#else
#include <semphr.h>
#endif

}

namespace estd {
    
namespace freertos {

namespace internal {

// DEBT: This is both a base class and a wrapper class.  Could be useful
// to make wrapper distinct.  When doing so, we'll decouple the auto deletion
// out of the wrapper
class semaphore_base
{
protected:
    const SemaphoreHandle_t s;

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

    BaseType_t give()
    {
        return xSemaphoreGive(s);
    }

    BaseType_t give_recursive()
    {
        return xSemaphoreGiveRecursive(s);
    }

    BaseType_t give_from_isr(BaseType_t* higherPriorityTaskWoken)
    {
        return xSemaphoreGiveFromISR(s, higherPriorityTaskWoken);
    }

    BaseType_t take(TickType_t ticksToWait)
    {
        return xSemaphoreTake(s, ticksToWait);
    }

    BaseType_t take_recursive(TickType_t ticksToWait)
    {
        return xSemaphoreTakeRecursive(s, ticksToWait);
    }

    BaseType_t take_from_isr(BaseType_t* higherPriorityTaskWoken)
    {
        return xSemaphoreTakeFromISR(s, higherPriorityTaskWoken);
    }

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
        take(portMAX_DELAY);
    }

    bool try_acquire()
    {
        return take(0);
    }

    void release()
    {
        give();
    }
};

}

#ifndef FEATURE_CPP_ALIASTEMPLATE
template <>
class binary_semaphore<false> : public internal::semaphore_base
{
public:
    binary_semaphore() :
        internal::semaphore_base(xSemaphoreCreateBinary())
    {}
};
#endif

// NOTE: Deviate from spec in that we have implicit '0' for desired,
// as well as unsigned instead of ptrdiff_t

template<unsigned max>
class counting_semaphore<max, false> : public internal::semaphore_base
{
public:
    counting_semaphore(unsigned desired = 0) :
        internal::semaphore_base(xSemaphoreCreateCounting(max, desired))
    {}
};

template <>
class counting_semaphore<1, false> : public internal::semaphore_base
{
public:
    counting_semaphore() :
        internal::semaphore_base(xSemaphoreCreateBinary())
    {}
};


template<unsigned max>
class counting_semaphore<max, true> : public internal::semaphore_base
{
    StaticSemaphore_t storage;

public:
    counting_semaphore(unsigned desired = 0) :
        internal::semaphore_base(
            xSemaphoreCreateCountingStatic(max, desired, &storage))
    {}
};

}

#if FEATURE_ESTD_FREERTOS_THREAD
template <unsigned max>
using counting_semaphore = freertos::counting_semaphore<max, false>;
#endif

}