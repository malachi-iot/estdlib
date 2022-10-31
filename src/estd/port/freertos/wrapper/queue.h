#pragma once

#if ESP_PLATFORM
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#else
#include <queue.h>
#endif

// Using estd naming conventions
namespace estd { namespace freertos {
    
namespace wrapper {

struct queue_base
{
protected:
    QueueHandle_t h;

    inline queue_base(QueueHandle_t h) : h{h} {}

public:
    ESTD_CPP_CONSTEXPR_RET operator QueueHandle_t () const { return h; }
};

struct queue : queue_base
{
    queue(QueueHandle_t h) : queue_base{h} {}

public:
    static queue create(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
    {
        queue q{xQueueCreate(uxQueueLength, uxItemSize)};
        return q;
    }

    static queue create(UBaseType_t uxQueueLength, UBaseType_t uxItemSize,
        uint8_t* pucQueueStorageBuffer, StaticQueue_t* pxQueueBuffer)
    {
        queue q{xQueueCreateStatic(uxQueueLength, uxItemSize, pucQueueStorageBuffer, pxQueueBuffer)};
        return q;
    }

#if ( configQUEUE_REGISTRY_SIZE > 0 )
    const char* get_name() const { return pcQueueGetName(h); }
#endif

    void reset() { xQueueReset(h); }

    void free() { vQueueDelete(h); }

    BaseType_t send_from_isr(const void* pvItemToQueue, BaseType_t* pxHigherPriorityTaskWoken)
    {
        return xQueueSendFromISR(h, pvItemToQueue, pxHigherPriorityTaskWoken);
    }

    BaseType_t receive(void* pvBuffer, TickType_t xTicksToWait)
    {
        return xQueueReceive(h, pvBuffer, xTicksToWait);
    }
};

}



}}