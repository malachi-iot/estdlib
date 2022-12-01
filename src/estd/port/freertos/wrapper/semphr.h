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
    struct binary_tag {};
    struct counting_tag {};
    struct recursive_mutex_tag {};
    struct mutex_tag {};

    semaphore() = default;
    semaphore(SemaphoreHandle_t s) :
        s{s} {}

    inline static SemaphoreHandle_t create(binary_tag)
    {
        return xSemaphoreCreateBinary();
    }

    inline static SemaphoreHandle_t create(counting_tag, UBaseType_t maxCount, UBaseType_t initialCount)
    {
        return xSemaphoreCreateCounting(maxCount, initialCount);
    }

#if configSUPPORT_STATIC_ALLOCATION
    inline static SemaphoreHandle_t create(binary_tag, StaticSemaphore_t* storage)
    {
        return xSemaphoreCreateBinaryStatic(storage);
    }

    inline static SemaphoreHandle_t create(counting_tag, UBaseType_t maxCount, UBaseType_t initialCount, StaticSemaphore_t* storage)
    {
        return xSemaphoreCreateCountingStatic(maxCount, initialCount, storage);
    }

    inline static SemaphoreHandle_t create(mutex_tag, StaticSemaphore_t* storage)
    {
        return xSemaphoreCreateMutexStatic(storage);
    }

    SemaphoreHandle_t create_binary(StaticSemaphore_t* storage)
    {
        return s = create(binary_tag(), storage);
    }
#endif

#if configSUPPORT_DYNAMIC_ALLOCATION
    inline static SemaphoreHandle_t create(mutex_tag)
    {
        return xSemaphoreCreateMutex();
    }

    SemaphoreHandle_t create_binary()
    {
        return s = create(binary_tag());
    }

    SemaphoreHandle_t create_counting(
        UBaseType_t maxCount, UBaseType_t initialCount)
    {
        return s = create(counting_tag(), maxCount, initialCount);
    }

    SemaphoreHandle_t create_mutex()
    {
        return s = xSemaphoreCreateMutex();
    }

    SemaphoreHandle_t create_recursive_mutex()
    {
        return s = xSemaphoreCreateRecursiveMutex();
    }
#endif // configSUPPORT_DYNAMIC_ALLOCATION

#if configSUPPORT_STATIC_ALLOCATION
    SemaphoreHandle_t create_counting(
        UBaseType_t maxCount, UBaseType_t initialCount,
        StaticSemaphore_t* storage)
    {
        return s = create(counting_tag(), maxCount, initialCount, storage);
    }

    SemaphoreHandle_t create_mutex(StaticSemaphore_t* storage)
    {
        return s = xSemaphoreCreateMutexStatic(storage);
    }

    SemaphoreHandle_t create_recursive_mutex(StaticSemaphore_t* storage)
    {
        return s = xSemaphoreCreateRecursiveMutexStatic(storage);
    }
#endif

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

    UBaseType_t count() const
    {
        return uxSemaphoreGetCount(s);
    }

    TaskHandle_t mutex_holder() const
    {
        return xSemaphoreGetMutexHolder(s);
    }

    // NOTE: A little sneaky, we can const this because underlying
    // 's' isn't written to.  However, it certainly gets invalidated
    void free() const
    {
        vSemaphoreDelete(s);
    }

    operator SemaphoreHandle_t () const
    {
        return s;
    }
};


}}}