#include <Arduino.h>

// FIX: Fix some platforms have the std headers inbuilt and some don't
// Arduino presence automatically assumes NOT but that was incorrect
// so temporarily disabling ARDUINO for this example so that we properly
// heed existing std headers
#undef ARDUINO

#include <estd/type_traits.h>
#include <estd/string.h>

// FIX: Now re-enabling to avoid any further issues
#define ARDUINO

// TODO: Move this out of example and into framework code
template<class TImpl> inline Print &operator <<(
    Print &obj, const estd::internal::dynamic_array<TImpl>& arg) 
{
    obj.write(arg.clock(), arg.size());
    arg.cunlock();
    return obj;
}

// Adapted from
// https://playground.arduino.cc/Main/StreamingOutput
// doing extra magic to exclude my own overload
template<class T, class Enable = 
    typename estd::enable_if<
        // FIX: totally cheating here - relying on the fact that our string
        // is a class, so excluding it from being picked up
        !estd::is_class<T>::value
        //!estd::is_base_of<estd::internal::no_max_string_length_tag, T>::value
    >::type
>
inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

void setup()
{
    Serial.begin(115200);

    estd::layer1::string<64> name;

    // FIX: Can't do this assignment, it confuses the compiler
    //name = "Mickey";

    Serial << F("Hello: ") << name;
}



void loop()
{

}