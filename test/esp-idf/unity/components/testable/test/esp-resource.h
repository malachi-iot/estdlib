#pragma once

#include <estd/span.h>

// Brought in from embr::gl - add a 'span' to this guy too

#define ESP_BINARY(variable_name, resource_name)    \
extern const uint8_t variable_name ## _start[] asm("_binary_" resource_name "_start");    \
extern const uint8_t variable_name ## _end[]   asm("_binary_" resource_name "_end");
