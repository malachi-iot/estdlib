#pragma once

#if ESP_PLATFORM
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#else
#include <timers.h>
#endif

#include <estd/port/freertos/chrono.h>

namespace estd { namespace freertos { 
    
namespace wrapper {

class timer
{
    TimerHandle_t h;

public:
    timer(TimerHandle_t h) : h(h) {}

#if configSUPPORT_DYNAMIC_ALLOCATION
    static timer create(const char* const pcTimerName,
        const TickType_t xTimerPeriod,
        const UBaseType_t uxAutoReload,
        void* const pvTimerID,
        TimerCallbackFunction_t pxCallbackFunction)
    {
        return xTimerCreate(
            pcTimerName, xTimerPeriod,
            uxAutoReload, pvTimerID,
            pxCallbackFunction);
    }
#endif

#if configSUPPORT_STATIC_ALLOCATION
    static timer create(const char* const pcTimerName,
        const TickType_t xTimerPeriod,
        const UBaseType_t uxAutoReload,
        void* const pvTimerID,
        TimerCallbackFunction_t pxCallbackFunction,
        StaticTimer_t* pxTimerBuffer)
    {
        return xTimerCreateStatic(
            pcTimerName, xTimerPeriod,
            uxAutoReload, pvTimerID,
            pxCallbackFunction,
            pxTimerBuffer);
    }
#endif

    BaseType_t free(TickType_t xBlockTime)
    {
        return xTimerDelete(h, xBlockTime);
    }

    BaseType_t start(TickType_t xBlockTime)
    {
        return xTimerStart(h, xBlockTime);
    }

    BaseType_t start_from_isr(BaseType_t* pxHigherPriorityTaskWoken)
    {
        return xTimerStartFromISR(h, pxHigherPriorityTaskWoken);
    }


    BaseType_t stop(TickType_t xBlockTime)
    {
        return xTimerStop(h, xBlockTime);
    }

    BaseType_t stop_from_isr(BaseType_t* pxHigherPriorityTaskWoken)
    {
        return xTimerStopFromISR(h, pxHigherPriorityTaskWoken);
    }

    void set_timer_id(void* pvNewId)
    {
        vTimerSetTimerID(h, pvNewId);
    }

    void* get_timer_id() const
    {
        return pvTimerGetTimerID(h);
    }

    const char* get_name() const
    {
        return pcTimerGetName(h);
    }

    BaseType_t change_period(TickType_t xNewPeriod, TickType_t xBlockTime)
    {
        return xTimerChangePeriod(h, xNewPeriod, xBlockTime);
    }

    TickType_t get_expiry_time() const
    {
        return xTimerGetExpiryTime(h);
    }

    TickType_t get_period() const
    {
        return xTimerGetPeriod(h);
    }

    BaseType_t reset(TickType_t xBlockTime)
    {
        return xTimerReset(h, xBlockTime);
    }

    BaseType_t reset_from_isr(BaseType_t* pxHigherPrioertyTaskWoken)
    {
        return xTimerResetFromISR(h, pxHigherPrioertyTaskWoken);
    }

    BaseType_t is_timer_active() const
    {
        return xTimerIsTimerActive(h);
    }

    static BaseType_t pend_function_call(PendedFunction_t xFunctionToPend,
        void* pvParameter1,
        uint32_t ulParameter2,
        TickType_t xTicksToWait)
    {
        return xTimerPendFunctionCall(xFunctionToPend, pvParameter1, ulParameter2, xTicksToWait);
    }

    static BaseType_t pend_function_call_from_isr(PendedFunction_t xFunctionToPend,
        void* pvParameter1,
        uint32_t ulParameter2,
        BaseType_t* pxHigherPriorityTaskWoken)
    {
        return xTimerPendFunctionCallFromISR(xFunctionToPend,
            pvParameter1, ulParameter2, pxHigherPriorityTaskWoken);
    }
};

}

namespace internal {

// Does no housekeeping, primarily sugars the API
class timer
{
    wrapper::timer t;

public:
    typedef estd::chrono::freertos_clock::duration duration;
    typedef estd::chrono::freertos_clock::time_point time_point;

    timer(const wrapper::timer& t) : t(t) {}

    bool dormant() const { return t.is_timer_active() == pdFALSE; }
    bool active() const { return !dormant(); }

    BaseType_t free(duration block_time)
    {
        return t.free(block_time.count());
    }

    const char* name() const { return t.get_name(); }

    void id(void* v) { t.set_timer_id(v); }
    void* id() const { return t.get_timer_id(); }

    BaseType_t period(duration new_period, duration block_time)
    {
        return t.change_period(new_period.count(), block_time.count());
    }

    duration period() const { return duration(t.get_period()); }

    BaseType_t start(duration block_time)
    {
        return t.start(block_time.count());
    }

    BaseType_t stop(duration block_time)
    {
        return t.stop(block_time.count());
    }

    BaseType_t reset(duration block_time)
    {
        return t.reset(block_time.count());
    }

    BaseType_t stop_from_isr(BaseType_t* pxHigherPrioertyTaskWoken)
    {
        return t.stop_from_isr(pxHigherPrioertyTaskWoken);
    }

    BaseType_t reset_from_isr(BaseType_t* pxHigherPrioertyTaskWoken)
    {
        return t.reset_from_isr(pxHigherPrioertyTaskWoken);
    }

    time_point expiry() const
    {
        return time_point(duration(t.get_expiry_time()));
    }


    wrapper::timer& native() { return t; }

    static BaseType_t pend_function_call(PendedFunction_t xFunctionToPend,
        void* pvParameter1,
        uint32_t ulParameter2,
        duration wait_time)
    {
        return xTimerPendFunctionCall(xFunctionToPend, pvParameter1, ulParameter2, wait_time.count());
    }
};

}

}}