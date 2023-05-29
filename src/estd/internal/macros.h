// These are utility wrappers for forward and backward compatibility
// C++11-ish wrapper section
#pragma once

#include "feature/std.h"
#include "feature/cpp.h"
#include "macro/c++11_emul.h"

// NOTE: Only applies to empty constructor.  You're on your own for the "more complex" varieties
#ifdef FEATURE_CPP_DEFAULT_CTOR
#define ESTD_CPP_DEFAULT_CTOR(class_name)   constexpr class_name() = default;
#else
#define ESTD_CPP_DEFAULT_CTOR(class_name)   class_name() {}
#endif

