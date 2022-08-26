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

#include "chrono.h"

namespace estd {

namespace freertos {

template <bool static_allocated>
class mutex;

template <>
class mutex<false>
{
    const SemaphoreHandle_t s;

protected:
    mutex(SemaphoreHandle_t s) : s(s) {}

public:
    typedef SemaphoreHandle_t native_handle_type;

    mutex(bool binary = false) :
        s(binary ? xSemaphoreCreateBinary() : xSemaphoreCreateMutex())
    {
    }

    ~mutex()
    {
        // From the source code:
        /* The queue could have been allocated statically or dynamically, so
         * check before attempting to free the memory. */
        // Therefore we use this for both static and dynamic allocations of
        // semaphore
        vSemaphoreDelete(s);
    }

    native_handle_type native_handle() const { return s; }

    void lock()
    {
        xSemaphoreTake(s, portMAX_DELAY);
    }

    bool try_lock()
    {
        return xSemaphoreTake(s, 0);
    }

    bool unlock()
    {
        return xSemaphoreGive(s) == pdTRUE;
    }
};

template <>
class mutex<true> : public mutex<false>
{
    typedef mutex<false> base_type;

protected:
    StaticSemaphore_t storage;

public:
    mutex(bool binary = false) :
        base_type(binary ?
            xSemaphoreCreateBinaryStatic(&storage) : 
            xSemaphoreCreateMutexStatic(&storage))
    {
    }
};


template <bool static_allocated>
class timed_mutex : public mutex<static_allocated>
{
    typedef mutex<static_allocated> base_type;

public:
    template< class Rep, class Period >
    bool try_lock_for( const estd::chrono::duration<Rep,Period>& timeout_duration )
    {
        // This should convert whatever incoming time format duration into our steady_clock
        // duration, which is very specifically freertos-tick bound
        // if this works well, use duration_cast instead and directly
        // on xSemaphoreTake
        estd::chrono::freertos_clock::duration d = timeout_duration;

        // get number of ticks
        return xSemaphoreTake(base_type::s, d.count());
    }

    template< class Clock, class Duration >
    bool try_lock_until( const estd::chrono::time_point<Clock,Duration>& timeout_time )
    {
        return xSemaphoreTake(base_type::s, timeout_time - chrono::freertos_clock::now());
    }
};

    
}

#if FEATURE_ESTD_FREERTOS_THREAD
typedef freertos::mutex<false> mutex;
typedef freertos::timed_mutex<false> timed_mutex;
#endif

}
