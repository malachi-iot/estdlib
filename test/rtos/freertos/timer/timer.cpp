#include <estd/port/freertos/timer.h>

using namespace estd::chrono;
using namespace estd::freertos;

#if defined(ESP_PLATFORM)

static void one_shot(TimerHandle_t t);
static void periodic(TimerHandle_t t);

#if configSUPPORT_DYNAMIC_ALLOCATION
static timer<false> dynamic_one_shot("dynamic one-shot", seconds(2), false, NULLPTR, one_shot);
static timer<false> dynamic_periodic("dynamic periodic", seconds(2), true, NULLPTR, periodic);
#endif

#if configSUPPORT_STATIC_ALLOCATION
static timer<true> static_one_shot("static one-shot", seconds(2), false, NULLPTR, one_shot);
static timer<true> static_periodic("static periodic", seconds(2), true, NULLPTR, periodic);
#endif


#include "esp_log.h"

static const char* TAG = "freertos::timer";

static void one_shot(TimerHandle_t t)
{
    estd::freertos::internal::timer timer(t);
    
    ESP_LOGI(TAG, "one_shot: %s", timer.name());

    timer.reset(milliseconds(100));
}

static void periodic(TimerHandle_t t)
{
    estd::freertos::internal::timer timer(t);

    ESP_LOGI(TAG, "periodic: %s", timer.name());
}

#endif


namespace freertos {

void timer_test()
{
    static_periodic.start(milliseconds(100));
    static_one_shot.start(milliseconds(100));
    dynamic_periodic.start(milliseconds(100));
    dynamic_one_shot.start(milliseconds(100));

    // TODO: Need versions which reset from ISR
    
    // DEBT: Test deallocations
    // DEBT: Guard for no timer availability at all + dynamic/static modes
}

}

