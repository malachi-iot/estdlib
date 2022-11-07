#pragma once

#include <estd/internal/platform.h>

#if ESP_PLATFORM
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#else
#include <event_groups.h>
#endif

namespace estd { namespace freertos { namespace wrapper {

class event_group
{
    const EventGroupHandle_t h;

public:
    ESTD_CPP_DEFAULT_CTOR(event_group)
    inline event_group(EventGroupHandle_t h) : h(h) {}

    static event_group create()
    {
        return xEventGroupCreate();
    }

    static event_group create_static(StaticEventGroup_t* pxEventGroupBuffer)
    {
        return xEventGroupCreateStatic(pxEventGroupBuffer);
    }

    void free()
    {
        vEventGroupDelete(h);
    }

    BaseType_t clear_bits_from_isr(const EventBits_t uxBitsToClear)
    {
        return xEventGroupClearBitsFromISR(h, uxBitsToClear);
    }

    EventBits_t wait_bits(
                       const EventBits_t uxBitsToWaitFor,
                       const BaseType_t xClearOnExit,
                       const BaseType_t xWaitForAllBits,
                       TickType_t xTicksToWait)
    {
        return xEventGroupWaitBits(h, uxBitsToWaitFor, xClearOnExit, xWaitForAllBits, xTicksToWait);
    }

    EventBits_t set_bits(const EventBits_t uxBitsToSet)
    {
        return xEventGroupSetBits(h, uxBitsToSet);
    }


    BaseType_t set_bits_from_isr(const EventBits_t uxBitsToSet,
                          BaseType_t* pxHigherPriorityTaskWoken)
    {
        return xEventGroupSetBitsFromISR(h, uxBitsToSet, pxHigherPriorityTaskWoken);
    }

    EventBits_t sync(const EventBits_t uxBitsToSet,
                              const EventBits_t uxBitsToWaitFor,
                              TickType_t xTicksToWait)
    {
        return xEventGroupSync(h, uxBitsToSet, uxBitsToWaitFor, xTicksToWait);
    }
};

}}}