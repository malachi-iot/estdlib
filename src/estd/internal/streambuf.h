#pragma once

#include "impl/streambuf/tags.h"

namespace estd { namespace internal {

// all the noop/defaults which we expect to hide/overload are now located in impl
// this struct now amounts to a type tag
struct streambuf_baseline { };

}}