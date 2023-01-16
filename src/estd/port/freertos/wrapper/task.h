#pragma once

#include "../../../internal/platform.h"
#include "../../arch/freertos.h"

extern "C" {

#if ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <task.h>
#endif

}

namespace estd { namespace freertos { namespace wrapper {

class task
{
    TaskHandle_t t;

public:
    task() = default;
    task(TaskHandle_t t) : t(t) {}

    // DEBT: Might be better to eliminate these and use  eAction=pdPASS
    BaseType_t notify_give() const
    {
        return xTaskNotifyGive(t);
    }

    // DEBT: Filter these out by ESTD_OS_FREERTOS version stamp 10.4.x
    BaseType_t notify_give(UBaseType_t indexToNotify) const
    {
        return xTaskNotifyGiveIndexed(t, indexToNotify);
    }

    void notify_give_from_isr(BaseType_t* higherPriorityTaskWoken = NULLPTR) const
    {
        vTaskNotifyGiveFromISR(t, higherPriorityTaskWoken);
    }

    BaseType_t notify(uint32_t ulValue, eNotifyAction eAction) const
    {
        return xTaskNotify(t, ulValue, eAction);
    }

    BaseType_t notify_from_isr(uint32_t ulValue, eNotifyAction eAction,
        BaseType_t* higherPriorityTaskWoken = NULLPTR) const
    {
        return xTaskNotifyFromISR(t, ulValue, eAction, higherPriorityTaskWoken);
    }

    uint32_t notify_value_clear(uint32_t bitsToClear) const
    {
        return ulTaskNotifyValueClear(t, bitsToClear);
    }

    BaseType_t notify_state_clear() const
    {
        return xTaskNotifyStateClear(t);
    }

    const char* name() const
    {
        return pcTaskGetName(t);
    }

#if configUSE_APPLICATION_TASK_TAG 
    TaskHookFunction_t tag() const
    {
        return xTaskGetApplicationTaskTag(t);
    }

    void tag(TaskHookFunction_t tagValue)
    {
        xTaskSetApplicationTaskTag(t, tagValue);
    }
#endif

    void info(TaskStatus_t* taskStatus, BaseType_t getFreeStackSpace, eTaskState eState) const
    {
        vTaskGetInfo(t, taskStatus, getFreeStackSpace, eState);
    }

    eTaskState state() const
    {
        return eTaskGetState(t);
    }

    static task current()
    {
        return task(xTaskGetCurrentTaskHandle());
    }

    operator TaskHandle_t () const
    {
        return t;
    }
};

}}}