#include "freertos/ringbuf.h"

#ifndef ESP_PLATFORM
#error "ring buffer is part of esp-idf FreeRTOS supplemental feature"
#endif

namespace estd { namespace freertos {

namespace wrapper {

struct ring_buffer
{
    RingbufHandle_t h;

    void create(size_t sz, RingbufferType_t type)
    {
        h = xRingbufferCreate(sz, type);
    }

    void create(size_t sz, RingbufferType_t type,
        uint8_t* pucRingbufferStorage,
        StaticRingbuffer_t* pxStaticRingbuffer)
    {
        h = xRingbufferCreateStatic(sz, type, pucRingbufferStorage, pxStaticRingbuffer);
    }

    void free()
    {
        vRingbufferDelete(h);
    }

    BaseType_t send_acquire(void **ppvItem, size_t xItemSize, TickType_t xTicksToWait)
    {
        return xRingbufferSendAcquire(h, ppvItem, xItemSize, xTicksToWait);
    }

    BaseType_t send_complete(void *pvItem)
    {
        return xRingbufferSendComplete(h, pvItem);
    }

    void* receive(size_t* sz,  TickType_t xTicksToWait)
    {
        return xRingbufferReceive(h, sz, xTicksToWait);
    }

    void return_item(void* pvItem)
    {
        vRingbufferReturnItem(h, pvItem);
    }

    operator RingbufHandle_t() const { return h; }
};

}

}}