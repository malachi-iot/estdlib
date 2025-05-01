#pragma once

#include <estd/span.h>

// Brought in from embr::gl - eventually move this out of 'unity' to general use

#define ESP_BINARY_ARRAY(variable_name, resource_name)  \
extern const uint8_t variable_name ## _start[] asm("_binary_" resource_name "_start");      \
extern const uint8_t variable_name ## _end[]   asm("_binary_" resource_name "_end");        \

#define ESP_BINARY_SPAN(variable_name)    \
estd::span<const uint8_t> variable_name(variable_name ## _start, variable_name ## _end);

#define ESP_BINARY(variable_name, resource_name)        \
ESP_BINARY_ARRAY(variable_name, resource_name)          \
ESP_BINARY_SPAN(variable_name)
