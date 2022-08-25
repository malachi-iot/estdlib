#if ESP_PLATFORM
#include "freertos/task.h"
#else
#include "task.h"
#endif

//#define ESTD_BUILD_IDF_SHORT_VER(major, minor, patch)  ((major * 10000) + (minor * 100) + patch)