#pragma once

#include "../fwd/string.h"

#if FEATURE_STD_CHARCONV
#include <charconv>
#endif

namespace estd {

namespace internal {

// 's' must be a null-terminated basic_string
template <class T, class Impl>
void to_string_float(detail::basic_string<Impl>& s, const T& value)
{
    char* raw = s.lock();
    // aligns with ios_base default
    // as per https://en.cppreference.com/w/cpp/io/ios_base/precision.html
    static constexpr unsigned precision = 6;

#if FEATURE_STD_CHARCONV
    constexpr std::chars_format format{std::chars_format::fixed};
    const std::to_chars_result r = std::to_chars(raw, raw + s.max_size() - 1,
        value, format, precision);

    *r.ptr = 0;
#elif __AVR__
    dtostrf(value, 6, precision, raw);
#else
    (void)raw;
    (void)precision;
    static_assert(!is_floating_point<T>::value, "Not yet supported");
#endif

    s.unlock();
}

}

}
