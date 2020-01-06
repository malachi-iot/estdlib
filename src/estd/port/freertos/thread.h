#pragma once

#include "chrono.h"
#include "../../functional.h"

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <task.h>
#endif


namespace estd {

class thread
{
public:
    typedef TaskHandle_t native_handle_type;
    // this will map to xTaskNumber from xTASK_STATUS
    // turns out this is only available from debug call to vTaskGetInfo
    //typedef UBaseType_t id;
    typedef native_handle_type id;

private:
#ifdef FEATURE_ESTD_EXP_FREERTOS_THREAD
    union
    {
        TaskHandle_t th;    // new task
#ifdef configUSE_TASK_NOTIFICATIONS
        TaskHandle_t hostTask;
#else
        SemaphoreHandle_t semaphore;
#endif
    };

    template <class Function, class ... Args>
    static void start_thread(void* p)
    {
        auto bound = (internal::bind_type<Function, Args...>*)p;
        auto bound_inline = std::move(*bound);

        //xSemaphoreGive(semaphore);

        // As per https://www.freertos.org/RTOS-task-notifications.html, this is the recommended way for
        // lightweight notifications
        xTaskNotifyGive(hostTask);

        bound_inline.operator ()();
    }

protected:
    thread(TaskHandle_t th) : th(th) {}

public:
    template <class Function, class ... Args>
    thread(Function&& f, Args&&... args)
    {
        internal::bind_type<Function, Args...> binding(
            std::move(f), 
            std::forward<Args>(args)...);

        //StaticSemaphore_t semaphoreStorage;

        //semaphore = xSemaphoreCreateBinaryStatic(&semaphoreStorage);

        hostTask = xTaskGetCurrentTaskHandle();

        TaskHandle_t th;
        BaseType_t status = xTaskCreate(start_thread<Function, Args...>,
            "test_task", 4096, &binding, 4, &th);

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        //xSemaphoreTake(semaphore);
    }

    ~thread()
    {
        // FIX: though this is std lib behavior, some people
        // suggest you should issue vTaskDelete from within the running
        // task itself
        vTaskDelete(th);
    }
    

    native_handle_type native_handle()
    {
        return th;
    }

    id get_id() const
    {
        return th;
    }
#endif
};

namespace this_thread {

// untested, not yet compiled, but everything is in place so I expect this to work with little
// effort
template< class Rep, class Period >
void sleep_for( const chrono::duration<Rep, Period>& sleep_duration )
{
    // TODO: Pretty sure we can do a straight chrono::stead_clock::duration instance here
    chrono::freertos_clock::rep count =
            estd::chrono::duration_cast<chrono::freertos_clock::duration>(sleep_duration).count();

    vTaskDelay(count);
}

// FreeRTOS' "delay until" code is subtly but fundamentally different than std lib version,
// so we don't wrap it
#ifdef INCLUDE_vTaskDelayUntil
#endif

// compiles, but untested
template< class Clock, class Duration >
void sleep_until( const chrono::time_point<Clock,Duration>& sleep_time )
{
    chrono::freertos_clock::rep count = sleep_time - chrono::freertos_clock::now();

    vTaskDelay(count);
}

inline void yield() { taskYIELD(); }

thread::id get_id()
{
    return xTaskGetCurrentTaskHandle();
}

}

}
