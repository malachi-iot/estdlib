/**
 * @file
 * NOTE: This file is auto included via platform.h -> supported_platform.h
 * calling chain, so don't bother including it directly yourself
 */
#pragma once

#include <estd/type_traits.h>

#ifndef ENABLE_ESTD_ARDUINO_STREAMING
#define ENABLE_ESTD_ARDUINO_STREAMING 1
#endif

#if ENABLE_ESTD_ARDUINO_STREAMING
// Adapted from
// https://playground.arduino.cc/Main/StreamingOutput
// doing extra magic to exclude my own overload
template<class T, class Enable = 
    typename estd::enable_if<
        // Exclude any specialized estd types known to have its own
        // << operator overload
        !estd::is_base_of<estd::internal::print_handler_tag, T>::value
    >::type
>
inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }
#endif
