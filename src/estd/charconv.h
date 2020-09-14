/*
 * @file
 */
#pragma once

#include "system_error.h"

namespace estd {

struct from_chars_result {
    const char* ptr;
    estd::errc  ec;

#if __cplusplus < 201103L
    from_chars_result(const char* ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}
#endif
};

from_chars_result from_chars(const char* first, const char* last,
                             long& value, int base = 10);

}