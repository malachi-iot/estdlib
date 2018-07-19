#pragma once

#include "../internal/platform.h"


// Only for scenarios where system has no std::char_traits to lean on
#ifndef FEATURE_STD_STRING

namespace std {

template<class CharT> class char_traits;

template<>
struct char_traits<char>
{
    typedef char char_type;
    typedef int int_type;
};

}

#endif
