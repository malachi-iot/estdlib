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

#if configUSE_TASK_NOTIFICATIONS

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
#endif

    const char* name() const
    {
        return pcTaskGetName(t);
    }

    void resume() const
    {
        vTaskResume(t);
    }

    void suspend() const
    {
        vTaskSuspend(t);
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

#if configUSE_TRACE_FACILITY 
    void info(TaskStatus_t* taskStatus, BaseType_t getFreeStackSpace, eTaskState eState) const
    {
        vTaskGetInfo(t, taskStatus, getFreeStackSpace, eState);
    }
#endif

    eTaskState state() const
    {
        return eTaskGetState(t);
    }

    static task current()
    {
        return task(xTaskGetCurrentTaskHandle());
    }

    // Being that we're a wrapper, non RAII is OK

#if configSUPPORT_DYNAMIC_ALLOCATION
    BaseType_t create(TaskFunction_t pvTaskCode,
        const char * const pcName,
        const configSTACK_DEPTH_TYPE uxStackDepth,
        void *pvParameters,
        UBaseType_t uxPriority)
    {
        return xTaskCreate(pvTaskCode, pcName, uxStackDepth, pvParameters, uxPriority, &t);
    }

#if portUSING_MPU_WRAPPERS
    BaseType_t create_restricted(TaskParameters_t *pxTaskDefinition)
    {
        return xTaskCreateRestricted(pxTaskDefinition, &t);
    }
#endif
#endif

#if configSUPPORT_STATIC_ALLOCATION
    // N represents byte count, not word count
    template <unsigned N>
    struct storage
    {
        static constexpr unsigned stack_depth = N / sizeof(StackType_t);

        StaticTask_t task;
        StackType_t stack[stack_depth];
    };

    static task create_static(TaskFunction_t pvTaskCode,
        const char* const pcName,
        const configSTACK_DEPTH_TYPE uxStackDepth,
        void* pvParameters,
        UBaseType_t uxPriority,
        StackType_t* const puxStackBuffer,
        StaticTask_t* const pxTaskBuffer)
    {
        return task(xTaskCreateStatic(pvTaskCode,
            pcName, uxStackDepth,
            pvParameters, uxPriority,
            puxStackBuffer,
            pxTaskBuffer));
    }

    template <unsigned N>
    static task create_static(TaskFunction_t pvTaskCode,
        const char* const pcName,
        void* pvParameters,
        UBaseType_t uxPriority,
        storage<N>* s)
    {
        // "The size of the task stack specified as the NUMBER OF BYTES. Note that this differs from vanilla FreeRTOS"
        // https://docs.espressif.com/projects/esp-idf/en/v5.3.2/esp32/api-reference/system/freertos_idf.html
        // We believe them.  The penalty for stack size inaccuracy is so high we do an assert
        // anyway.
#if ESP_PLATFORM
        static_assert(N == storage<N>::stack_depth, "ESP-IDF indicates stack depth is bytes");
#endif

        return task(xTaskCreateStatic(pvTaskCode,
            pcName,
            storage<N>::stack_depth,
            pvParameters, uxPriority,
            s->stack,
            &s->task));
    }

#endif

    operator TaskHandle_t () const
    {
        return t;
    }
};

}}}