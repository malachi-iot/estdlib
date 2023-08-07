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

#if configSUPPORT_STATIC_ALLOCATION
    static queue create(UBaseType_t uxQueueLength, UBaseType_t uxItemSize,
        uint8_t* pucQueueStorageBuffer, StaticQueue_t* pxQueueBuffer)
    {
        queue q{xQueueCreateStatic(uxQueueLength, uxItemSize, pucQueueStorageBuffer, pxQueueBuffer)};
        return q;
    }
#endif

#if ( configQUEUE_REGISTRY_SIZE > 0 )
    const char* get_name() const { return pcQueueGetName(h); }
#endif

    void free() { vQueueDelete(h); }

    UBaseType_t messages_waiting() const
    {
        return uxQueueMessagesWaiting(h);
    }

    BaseType_t overwrite(void* pvBuffer)
    {
        return xQueueOverwrite(h, pvBuffer);
    }

    BaseType_t overwrite_from_isr(void* pvBuffer,
        BaseType_t* pxHigherPriorityTaskWoken = nullptr)
    {
        return xQueueOverwriteFromISR(h, pvBuffer, pxHigherPriorityTaskWoken);
    }

    BaseType_t peek(void* pvBuffer, TickType_t xTicksToWait) const
    {
        return xQueuePeek(h, pvBuffer, xTicksToWait);
    }

    BaseType_t peek_from_isr(void* pvBuffer) const
    {
        return xQueuePeekFromISR(h, pvBuffer);
    }

    BaseType_t send(const void* pvItemToQueue, TickType_t xTicksToWait)
    {
        return xQueueSend(h, pvItemToQueue, xTicksToWait);
    }

    BaseType_t send_from_isr(const void* pvItemToQueue, BaseType_t* pxHigherPriorityTaskWoken)
    {
        return xQueueSendFromISR(h, pvItemToQueue, pxHigherPriorityTaskWoken);
    }

    BaseType_t send_to_back(const void* pvItemToQueue, TickType_t xTicksToWait)
    {
        return xQueueSendToBack(h, pvItemToQueue, xTicksToWait);
    }

    BaseType_t receive(void* pvBuffer, TickType_t xTicksToWait)
    {
        return xQueueReceive(h, pvBuffer, xTicksToWait);
    }

    BaseType_t reset()
    {
        return xQueueReset(h);
    }

    UBaseType_t spaces_available() const
    {
        return uxQueueSpacesAvailable(h);
    }
};

}



}}