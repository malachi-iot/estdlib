#pragma once

// mainly serves as aliases to std thread

#include "../../internal/platform.h"

#ifdef FEATURE_STD_THREAD

#include "../std/thread.h"

#else
namespace estd { namespace this_thread { 

inline void yield() NOEXCEPT
{
#warning Platform has no yield support.  Task/cpu usage may behave in unexpecteed ways
}

}}

#endif