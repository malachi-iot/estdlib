#pragma once

extern "C" {

#ifdef ESP32
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#else
#include <semphr.h>
#endif

}

#include "chrono.h"

namespace estd {

namespace experimental {

class mutex
{
    SemaphoreHandle_t s;

protected:
    mutex(SemaphoreHandle_t s) : s(s) {}

public:
    typedef SemaphoreHandle_t native_handle_type;

    mutex(bool binary = false)
    {
        if(binary)
            s = xSemaphoreCreateBinary();
        else
            s = xSemaphoreCreateMutex();
    }

    ~mutex()
    {
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

    void unlock()
    {
        xSemaphoreGive(s);
    }
};


class timed_mutex : public mutex
{
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
        return xSemaphoreTake(s, d.count());
    }

    template< class Clock, class Duration >
    bool try_lock_until( const estd::chrono::time_point<Clock,Duration>& timeout_time )
    {
        return xSemaphoreTake(s, timeout_time - chrono::freertos_clock::now());
    }
};


}
}
