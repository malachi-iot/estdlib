#pragma once

// for size_t typedef
#include <stdlib.h>

namespace estd {

// "a lightweight proxy object that provides access to an array of objects
//  of type const T"
// "not to be confused with 'member initializer list'"
// http://en.cppreference.com/w/cpp/utility/initializer_list
template <class T>
class initializer_list
{
public:
    typedef T value_type;
    typedef const T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
};

}