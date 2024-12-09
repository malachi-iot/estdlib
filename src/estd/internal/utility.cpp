#include <stdarg.h>
#include <stdio.h>

#include "platform.h"
#include "utility.h"

#include "../port/esp-idf/fake/esp_log.h"

#ifndef FEATURE_CPP_INLINE_STATIC
namespace estd { namespace internal {

CONSTEXPR has_member_base::yes has_member_base::yes_value = { 0 };
CONSTEXPR has_member_base::no has_member_base::no_value = { { 0 }, { 1 } };

}}
#endif

// ESP-IDF build chain doesn't pick this file up anyway, so this is just an extra protection
// in case we change our mind about that
#ifndef ESP_PLATFORM

static vprintf_like_t esp_log_writer = vprintf;
static va_list empty_list{};

vprintf_like_t esp_log_set_vprintf(vprintf_like_t func)
{
    // TODO: Consider swap operation here
    vprintf_like_t previous = esp_log_writer;
    esp_log_writer = func;
    return previous;
}

// DEBT: Consolidate this elsewhere IIRC J1939 lib has a c++ ESP-IDF logger helper
// (that one is for ostream)
static void esp_log_write_header(esp_log_level_t level, const char* tag)
{
    // DEBT: Do proper multi parameter write

    switch(level)
    {
        case ESP_LOG_ERROR:     esp_log_writer("E ", empty_list);   break;
        case ESP_LOG_WARN:      esp_log_writer("W ", empty_list);   break;
        case ESP_LOG_INFO:      esp_log_writer("I ", empty_list);   break;
        case ESP_LOG_DEBUG:     esp_log_writer("D ", empty_list);   break;
        case ESP_LOG_VERBOSE:   esp_log_writer("V ", empty_list);   break;
        default:                break;
    }

    esp_log_writer(tag, empty_list);
    esp_log_writer(": ", empty_list);
}

void esp_log_write(esp_log_level_t level, const char* tag, const char* format, ...)
{
    if(level <= LOG_LOCAL_LEVEL)
    {
        esp_log_write_header(level, tag);

        va_list args;
        va_start(args, format);
        esp_log_writer(format, args);
        va_end(args);

        esp_log_writer("\n", empty_list);
    }
}

#endif
