/*
 * @file
 */
#pragma once

#include "system_error.h"

namespace estd {

struct from_chars_result {
    const char* ptr;
    estd::errc  ec;
};

}