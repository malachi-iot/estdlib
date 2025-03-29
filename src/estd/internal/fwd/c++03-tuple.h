#pragma once

#include "../platform.h"
#include "../fwd/functional.h"
#include "../raw/variant.h"
#include "tuple-shared.h"

namespace estd {

// 29MAR25 MB DEBT: This is to create a partial coexistence between c++03 approach
// and c++11 approach.  However, c++03 support is basically phased out, and this feature
// flag is pretty clumsy - so both ought to go away
#ifndef FEATURE_ESTD_CPP03_TUPLE
#define FEATURE_ESTD_CPP03_TUPLE 1
#endif

#if FEATURE_ESTD_CPP03_TUPLE
template<class TArg1, class = monostate, class = monostate, class = monostate, class = monostate>
class tuple;
#endif



}