#pragma once

#include "read.h"

// Shamelessly copied from https://github.com/modm-io/avr-libstdcpp/blob/master/include/initializer_list
// who themselves shamelessly copied from GCC.  Viva open source!
#include "initializer_list.h"

namespace estd {

inline namespace v0 { inline namespace avr {

template <class T, unsigned N>
class test_container
{
private:
    const T data_[N];

public:
    // https://stackoverflow.com/questions/5549524/how-do-i-initialize-a-member-array-with-an-initializer-list

    /*
    constexpr test_container(std::initializer_list<T> l) :                                                                                                                  
        data_{l}
    {}  */
    template <typename... T2>
    constexpr test_container(T2... ts) : data_{ts...}
    {

    }

    T operator[](unsigned idx) const
    {
        return (T) internal::pgm_read<T>(data_ + (idx * sizeof(T)));
    }
};

}}

}