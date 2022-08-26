#pragma once

#include "../../arch/freertos.h"

extern "C" {

#if ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#else
#include <semphr.h>
#endif

}

namespace estd { namespace freertos { namespace wrapper {

class semaphore
{
protected:
    SemaphoreHandle_t s;

public:
    semaphore() = default;
    semaphore(SemaphoreHandle_t s) :
        s{s} {}

    BaseType_t give() const
    {
        return xSemaphoreGive(s);
    }

    BaseType_t give_recursive() const
    {
        return xSemaphoreGiveRecursive(s);
    }

    BaseType_t give_from_isr(BaseType_t* higherPriorityTaskWoken) const
    {
        return xSemaphoreGiveFromISR(s, higherPriorityTaskWoken);
    }

    BaseType_t take(TickType_t ticksToWait) const
    {
        return xSemaphoreTake(s, ticksToWait);
    }

    BaseType_t take_recursive(TickType_t ticksToWait) const
    {
        return xSemaphoreTakeRecursive(s, ticksToWait);
    }

    BaseType_t take_from_isr(BaseType_t* higherPriorityTaskWoken) const
    {
        return xSemaphoreTakeFromISR(s, higherPriorityTaskWoken);
    }

    operator SemaphoreHandle_t () const
    {
        return s;
    }
};


}}}