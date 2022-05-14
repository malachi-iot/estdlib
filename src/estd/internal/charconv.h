#pragma once

#include "../system_error.h"
#include "deduce_fixed_size.h"
#include "text/encodings.h"

namespace estd {

struct from_chars_result
{
    const char* ptr;
    estd::errc ec;

#ifndef __cpp_initializer_lists
    from_chars_result(const char* ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}
#endif
};

struct to_chars_result
{
    char* ptr;
    estd::errc ec;

#ifndef __cpp_initializer_lists
    to_chars_result(char* ptr, estd::errc ec) :
        ptr(ptr), ec(ec) {}
#endif
};



}
