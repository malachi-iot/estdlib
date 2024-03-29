#pragma once

#include "fwd.h"
#include "streambuf.h"
#include "../../../internal/ostream.h"

namespace estd {

template <class TChar, stdio_driver_t* d = nullptr, class TTraits = estd::char_traits<TChar>>
using basic_pico_ostream = estd::detail::basic_ostream<estd::basic_pico_stdio_streambuf<TChar, d, TTraits> >;

typedef basic_pico_ostream<char> pico_ostream;

}