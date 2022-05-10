#pragma once

#include "streambuf.h"
#include "../../internal/istream.h"

namespace estd {
    
typedef estd::internal::flagged_istream<estd::arduino_streambuf,
    estd::internal::istream_flags::blocking> arduino_istream;

namespace internal {

}

}