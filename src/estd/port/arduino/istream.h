#pragma once

#include "streambuf.h"
#include "../../internal/istream.h"

namespace estd {
    
typedef estd::internal::basic_istream<estd::arduino_streambuf> arduino_istream;

namespace internal {

}

}