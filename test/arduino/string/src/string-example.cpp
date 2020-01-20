#include <Arduino.h>

#include <estd/type_traits.h>
#include <estd/string.h>

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

void setup()
{
    Serial.begin(115200);

    estd::layer1::string<64> name;

    name = "Mickey";

    // these seem to irritate the compiler actually
    //Serial << F("Hello: ");
    //Serial.println(F("hi"));
    Serial << "Hello: ";
    Serial << name;
}



void loop()
{

}