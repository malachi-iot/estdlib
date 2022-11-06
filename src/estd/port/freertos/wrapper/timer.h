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

    TickType_t get_period() const
    {
        return xTimerGetPeriod(h);
    }

    BaseType_t reset(TickType_t xBlockTime)
    {
        return xTimerReset(h, xBlockTime);
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

class timer
{
    wrapper::timer t;

protected:
    typedef estd::chrono::freertos_clock::duration duration;

    timer(const wrapper::timer& t) : t(t) {}

public:
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

    BaseType_t reset(duration block_time)
    {
        return t.reset(block_time.count());
    }

    wrapper::timer& native() { return t; }
};

}

template <bool is_static = false>
class timer;

template <>
class timer<false> : public internal::timer
{
    typedef internal::timer base_type;

public:
    timer(const char* const name, duration period, bool auto_reload, void* const id,
        TimerCallbackFunction_t callback) :
        base_type(wrapper::timer::create(name, period.count(), auto_reload, id, callback))
    {

    }

    ~timer()
    {
        // DEBT: Feed this via a policy
        free(estd::chrono::milliseconds(100));
    }
};


// FIX: Not well tested, because unit test crashes when trying it.  Probably
// unit test frees its stack-allocated StaticTimer_t before timer is truly done with it
template <>
class timer<true> : public internal::timer
{
    typedef internal::timer base_type;

    StaticTimer_t storage;

public:
    timer(const char* const name, duration period, bool auto_reload, void* const id,
        TimerCallbackFunction_t callback) :
        base_type(wrapper::timer::create(name, period.count(), auto_reload, id, callback, &storage))
    {

    }

    ~timer()
    {
        // DEBT: Feed this via a policy
        free(estd::chrono::milliseconds(100));
    }
};


}}