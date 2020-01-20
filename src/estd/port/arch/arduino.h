#pragma once

// For all Arduino scenarios, we want print_handler_tag around
#include <Arduino.h>

namespace estd { namespace internal {

// Specialized tag to help with:
// https://playground.arduino.cc/Main/StreamingOutput/
// Specifically, if a type with this tag is encountered, we have our own
// specialized Stream& output handler
struct print_handler_tag {};

}}
