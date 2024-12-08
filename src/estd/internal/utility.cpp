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

vprintf_like_t esp_log_set_vprintf(vprintf_like_t func)
{
    // TODO: Consider swap operation here
    vprintf_like_t previous = esp_log_writer;
    esp_log_writer = func;
    return previous;
}

void esp_log_write(esp_log_level_t level, const char* tag, const char* format, ...)
{
    if(level <= LOG_LOCAL_LEVEL)
    {
        static va_list empty_list{};

        // TODO: Write out tag

        va_list args;
        va_start(args, format);
        esp_log_writer(format, args);
        va_end(args);

        //esp_log_writer("\n", empty_list);
    }
}

#endif