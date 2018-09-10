#pragma once

#include "chrono.h"
#include "../../functional.h"

#ifdef ESP32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <task.h>
#endif


namespace estd {

class thread
{
    TaskHandle_t th;

    template <class Function, class ... Args>
    static void start_thread(void* p)
    {
        auto bound = (internal::bind_type<Function, Args...>*)p;

        bound->operator ()();
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

        th = xTaskCreate(start_thread<Function, Args...>, 
            "test_task", 4096, NULL, 4, &th);
    }

    ~thread()
    {
        // FIX: though this is std lib behavior, some people
        // suggest you should issue vTaskDelete from within the running
        // task itself
        vTaskDelete(th);
    }
    

    typedef TaskHandle_t native_handle_type;
    // this will map to xTaskNumber from xTASK_STATUS
    // turns out this is only available from debug call to vTaskGetInfo
    //typedef UBaseType_t id;
    typedef native_handle_type id;

    native_handle_type native_handle()
    {
        return th;
    }

    id get_id() const
    {
        return th;
    }
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

inline void yield() { taskYIELD(); }

thread::id get_id()
{
    return xTaskGetCurrentTaskHandle();
}

}

}
