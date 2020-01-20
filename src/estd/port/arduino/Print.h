#pragma once

#ifndef ENABLE_ESTD_ARDUINO_STREAMING
#define ENABLE_ESTD_ARDUINO_STREAMING 1
#endif

#if ENABLE_ESTD_ARDUINO_STREAMING
// Adapted from
// https://playground.arduino.cc/Main/StreamingOutput
// doing extra magic to exclude my own overload
template<class T, class Enable = 
    typename estd::enable_if<
        // FIX: totally cheating here - relying on the fact that our string
        // is a class, so excluding it from being picked up
        //!estd::is_class<estd::internal::dynamic_array<TImpl> >::value
        !estd::is_base_of<estd::internal::print_handler_tag, T>::value
    >::type
>
inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }
#endif

