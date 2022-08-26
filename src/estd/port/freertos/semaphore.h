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

// DEBT: Consider consolidating with mutex_base
class semaphore_base
{
protected:
    const SemaphoreHandle_t s;

    semaphore_base(SemaphoreHandle_t s) :
        s{s} {}

    ~semaphore_base()
    {
        vSemaphoreDelete(s);
    }

    void acquire()
    {
        xSemaphoreTake(s, portMAX_DELAY);
    }

    bool try_acquire()
    {
        return xSemaphoreTake(s, 0);
    }

    void release()
    {
        xSemaphoreGive(s);
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