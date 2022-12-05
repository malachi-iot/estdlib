#include <estd/port/freertos/timer.h>

using namespace estd::chrono;
using namespace estd::freertos;

static constexpr milliseconds timeout(100);

static void one_shot(TimerHandle_t t);
static void periodic(TimerHandle_t t);

struct id
{
    bool isr;
};

static id _id1{false};
static id id_isr{true};

#if configSUPPORT_DYNAMIC_ALLOCATION
static timer<false> dynamic_one_shot("dynamic one-shot", seconds(2), false, &_id1, one_shot);
static timer<false> dynamic_periodic("dynamic periodic", seconds(2), true, &_id1, periodic);
#endif

#if configSUPPORT_STATIC_ALLOCATION
static timer<true> static_one_shot("static one-shot", seconds(2), false, &_id1, one_shot);
static timer<true> static_periodic("static periodic", seconds(2), true, &_id1, periodic);
static timer<true> static_one_shot_isr("dynamic one-shot (isr)", seconds(2), false, &id_isr, one_shot);
#endif


#if defined(ESP_PLATFORM)

#include "esp_log.h"

static const char* TAG = "freertos::timer";

static volatile int counter = 20;

static void one_shot(TimerHandle_t t)
{
    estd::freertos::internal::timer timer(t);
    
    if(counter > 0)
    {
        id* _id = (id*)timer.id();

        --counter;

        ESP_LOGI(TAG, "one_shot: %s, counter=%d, test_isr=%u", timer.name(), counter, _id->isr);

        if(_id->isr)
        {
            // NOTE: I am not 100% sure it's OK to call this outside an ISR, but I imagine it is supported.
            // Passes tests anyway
            BaseType_t pxHigherPriorityTaskWoken;
            timer.reset_from_isr(&pxHigherPriorityTaskWoken);
        }
        else
            timer.reset(timeout);
    }
}

static void periodic(TimerHandle_t t)
{
    estd::freertos::internal::timer timer(t);

    ESP_LOGI(TAG, "periodic: %s", timer.name());
}

#endif


namespace freertos {

// DEBT: For all of these, assert that operations were successful

volatile int* timer_test_begin()
{
#if configUSE_TIMERS 
#if configSUPPORT_STATIC_ALLOCATION
    static_periodic.start(timeout);
    static_one_shot.start(timeout);
    static_one_shot_isr.start(timeout);
#endif
#if configSUPPORT_DYNAMIC_ALLOCATION
    dynamic_periodic.start(timeout);
    dynamic_one_shot.start(timeout);
#endif
#else
#warn timer_test() skipped since timers are not enabled
    counter = 0;
#endif
    // TODO: Need versions which reset from ISR

    return &counter;
}


void timer_test_end()
{
#if configSUPPORT_STATIC_ALLOCATION
    static_periodic.stop(timeout);
    static_one_shot.stop(timeout);
    static_one_shot_isr.stop(timeout);

    static_periodic.~timer<true>();
    static_one_shot.~timer<true>();
    static_one_shot_isr.~timer<true>();

#endif
#if configSUPPORT_DYNAMIC_ALLOCATION
    dynamic_periodic.stop(timeout);
    dynamic_one_shot.stop(timeout);

    dynamic_periodic.~timer<false>();
    dynamic_one_shot.~timer<false>();
#endif
}

}

