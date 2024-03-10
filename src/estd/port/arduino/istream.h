#pragma once

#include "streambuf.h"
#include "../../internal/istream.h"

namespace estd {

// DEBT: Revise to use FEATURE_ESTD_STREAMBUF_TRAITS
typedef estd::internal::flagged_istream<estd::arduino_streambuf,
    estd::internal::istream_flags::blocking> arduino_istream;

namespace internal {

}

}