#pragma once

#include "istream.h"
#include "ostream.h"

namespace estd {

// UNTESTED, but compiles
using arduino_iostream = 
    detail::basic_ostream<estd::arduino_ostreambuf, arduino_istream>;

}