#pragma once

#include "streambuf.h"
#include "../../../internal/ostream.h"

namespace estd {

template <class TChar, stdio_driver_t* d, class TTraits = estd::char_traits<TChar>>
using basic_pico_ostream = estd::internal::basic_ostream<estd::layer0::basic_pico_stdio_streambuf<TChar, d, TTraits> >;

}