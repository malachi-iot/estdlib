#pragma once

#if defined(ARDUINO)
#define ESTD_ARDUINO
#elif defined(FREERTOS)
#define ESTD_FREERTOS
#endif
