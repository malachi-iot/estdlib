#pragma once

#include "streambuf.h"
#include "../../../internal/ostream.h"

namespace estd {

template <class TChar, stdio_driver_t* d>
using basic_pico_ostream = estd::internal::basic_ostream<estd::basic_pico_streambuf<TChar, d> >;

}